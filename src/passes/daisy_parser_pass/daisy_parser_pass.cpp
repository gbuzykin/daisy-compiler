#include "daisy_parser_pass.h"

#include "logger.h"
#include "text_utils.h"
#include "uxs/io/filebuf.h"
#include "uxs/stringalg.h"

#include <filesystem>

namespace lex_detail {
#include "lex_analyzer.inl"
}

namespace parser_detail {
#include "parser_analyzer.inl"
}

using namespace daisy;

DAISY_ADD_PASS(DaisyParserPass);

/*static*/ const ReduceActionHandler* ReduceActionHandler::first_avail = nullptr;
/*static*/ const PreprocDirectiveParser* PreprocDirectiveParser::first_avail = nullptr;

void DaisyParserPass::configure() {
    keywords_.insert({
        {"const", parser_detail::tt_const},
    });

    reduce_action_handlers_.fill(nullptr);
    for (const auto* handler = ReduceActionHandler::first_avail; handler; handler = handler->next_avail) {
        reduce_action_handlers_[handler->act_id] = handler->func;
    }

    for (const auto* parser = PreprocDirectiveParser::first_avail; parser; parser = parser->next_avail) {
        preproc_directive_parsers_[parser->directive_id] = parser;
    }
}

void DaisyParserPass::cleanup() {
    input_ctx_stack_.clear();
    lex_state_stack_.clear();
}

PassResult DaisyParserPass::run(CompilationContext& ctx) {
    uxs::basic_inline_dynbuffer<int, 1> parser_state_stack;
    std::vector<SymbolInfo> symbol_stack;

    ctx_ = &ctx;
    error_status_ = 0;
    lex_state_stack_.reserve_at_curr(256);
    parser_state_stack.reserve_at_curr(1024);
    symbol_stack.reserve(1024);

    // Load source file
    const auto* src_file = loadInputFile(ctx.file_name);
    if (!src_file) {
        logger::fatal().format("could not open input file `{}`", ctx.file_name);
        return PassResult::kFatalError;
    }

    // Create main source file input context
    pushInputContext(std::make_unique<InputContext>(src_file->getText(), &newLocationContext(src_file, {})));
    lex_state_stack_.push_back(lex_detail::sc_initial);

    // Parse input file
    int tt = lex(la_tkn_);
    parser_state_stack.push_back(parser_detail::sc_initial);  // Push initial state
    while (true) {
        int* prev_parser_state_stack_top = parser_state_stack.curr();
        parser_state_stack.reserve_at_curr(1);
        int act = parser_detail::parse(tt, parser_state_stack.first(), parser_state_stack.p_curr(), 0);
        if (act != parser_detail::predef_act_shift) {
            unsigned rlen = static_cast<unsigned>(1 + prev_parser_state_stack_top - parser_state_stack.curr());
            if (act < 0) {  // Syntax error
                // Successfully accept 3 tokens before the next error logging
                const int kAcceptToRestore = 3;
                if (!error_status_) { logSyntaxError(tt, la_tkn_.loc); }
                if (parser_state_stack.empty()) { return PassResult::kError; }
                if (error_status_ == kAcceptToRestore) {  // Discard lookahead symbol
                    if (tt == parser_detail::tt_end_of_file) { return PassResult::kError; }
                    symbol_stack.emplace_back(std::move(la_tkn_));
                    tt = lex(la_tkn_);
                    ++rlen;
                }
                error_status_ = kAcceptToRestore;
            }
            if (rlen == 0) { symbol_stack.emplace_back().loc = la_tkn_.loc, ++rlen; }  // Empty production workaround
            if (act > parser_detail::predef_act_reduce && reduce_action_handlers_[act]) {
                reduce_action_handlers_[act](this, &*(symbol_stack.end() - rlen));
            }
            if (rlen > 1) {
                (symbol_stack.end() - rlen)->loc += (symbol_stack.end() - 1)->loc;  // Default reduction location
                symbol_stack.erase(symbol_stack.end() - rlen + 1, symbol_stack.end());
            }
        } else if (tt != parser_detail::tt_end_of_file) {
            if (logger::g_debug_level >= 3) {
                if (tt == parser_detail::tt_id) {
                    logger::debugUnwind(la_tkn_.loc).format("id: {}", std::get<std::string_view>(la_tkn_.val));
                } else if (tt == parser_detail::tt_string_literal) {
                    logger::debugUnwind(la_tkn_.loc)
                        .format("string: {}", uxs::make_quoted_text(std::get<std::string>(la_tkn_.val)));
                } else if (tt == parser_detail::tt_int_literal) {
                    if (std::get<IntegerConst>(la_tkn_.val).isSigned()) {
                        logger::debugUnwind(la_tkn_.loc)
                            .format("integer number: {}", std::get<IntegerConst>(la_tkn_.val).getValue<int64_t>());
                    } else {
                        logger::debugUnwind(la_tkn_.loc)
                            .format("integer number: {}", std::get<IntegerConst>(la_tkn_.val).getValue<uint64_t>());
                    }
                } else if (tt == parser_detail::tt_real_literal) {
                    logger::debugUnwind(la_tkn_.loc).format("real number: {}", std::get<double>(la_tkn_.val));
                } else {
                    logger::debugUnwind(la_tkn_.loc).format("token");
                }
            }
            if (std::holds_alternative<std::string_view>(la_tkn_.val)) {
                la_tkn_.val = std::string(std::get<std::string_view>(la_tkn_.val));
            }
            symbol_stack.emplace_back(std::move(la_tkn_));
            tt = lex(la_tkn_);
            if (error_status_) { --error_status_; }
        } else {
            break;
        }
    }

    return ctx.error_count == 0 ? PassResult::kSuccess : PassResult::kError;
}

int DaisyParserPass::lex(SymbolInfo& tkn) {
    std::string txt;
    auto* in_ctx = &getInputContext();

    auto reset_token_loc = [&tkn](const auto& in_ctx) {
        tkn.loc.loc_ctx = in_ctx.loc_ctx;
        tkn.loc.first = in_ctx.text.pos;
    };

    reset_token_loc(*in_ctx);

    while (true) {
        int pat = 0;
        unsigned llen = 0;
        const char *first = in_ctx->text.first, *lexeme = first;
        while (true) {
            bool stack_limitation = false;
            const char* last = in_ctx->text.last;
            if (lex_state_stack_.avail() < static_cast<size_t>(last - first)) {
                last = first + lex_state_stack_.avail();
                stack_limitation = true;
            }
            assert(first <= last);
            pat = lex_detail::lex(first, last, lex_state_stack_.p_curr(), &llen, stack_limitation);
            if (pat >= lex_detail::predef_pat_default) {
                break;
            } else if (stack_limitation) {
                // enlarge state stack and continue analysis
                lex_state_stack_.reserve_at_curr(llen);
                first = last;
            } else {  // Input buffer is over
                if (lex_state_stack_.back() == lex_detail::sc_string) {
                    logger::warning(tkn.loc).format("unterminated string literal");
                    tkn.val = std::move(txt);
                    lex_state_stack_.pop_back();
                    return parser_detail::tt_string_literal;
                }
                tkn.loc.last = tkn.loc.first = in_ctx->text.pos;
                if (!!(in_ctx->flags & InputContext::Flags::kStopAtEndOfInput)) {
                    return parser_detail::tt_end_of_input;
                }
                // Input context stack is empty - end of compilation unit
                if (popInputContext()) { return parser_detail::tt_end_of_file; }
                reset_token_loc(*(in_ctx = &getInputContext()));
                // Proceed to the next token
                first = lexeme = in_ctx->text.first;
            }
        }

        in_ctx->text.first += llen, in_ctx->text.pos.col += llen;
        tkn.loc.last = {in_ctx->text.pos.ln, in_ctx->text.pos.col - 1};

        switch (pat) {
            // ------ escape sequences
            case lex_detail::pat_escape_a: txt.push_back('\a'); break;
            case lex_detail::pat_escape_b: txt.push_back('\b'); break;
            case lex_detail::pat_escape_f: txt.push_back('\f'); break;
            case lex_detail::pat_escape_r: txt.push_back('\r'); break;
            case lex_detail::pat_escape_n: txt.push_back('\n'); break;
            case lex_detail::pat_escape_t: txt.push_back('\t'); break;
            case lex_detail::pat_escape_v: txt.push_back('\v'); break;
            case lex_detail::pat_escape_other: {
                logger::warning(tkn.loc).format("unknown escape sequence");
                txt.push_back(lexeme[1]);
            } break;
            case lex_detail::pat_escape_hex: {
                char escape = uxs::dig_v<16>(lexeme[2]);
                if (llen > 3) { escape = (escape << 4) + uxs::dig_v<16>(lexeme[3]); }
                txt.push_back(escape);
            } break;
            case lex_detail::pat_escape_oct: {
                char escape = uxs::dig_v<8>(lexeme[1]);
                if (llen > 2) { escape = (escape << 3) + uxs::dig_v<8>(lexeme[2]); }
                if (llen > 3) { escape = (escape << 3) + uxs::dig_v<8>(lexeme[3]); }
                txt.push_back(escape);
            } break;

            // ------ operators
            case lex_detail::pat_inc: return parser_detail::tt_inc;
            case lex_detail::pat_dec: return parser_detail::tt_dec;
            case lex_detail::pat_shl: return parser_detail::tt_shl;
            case lex_detail::pat_shr: return parser_detail::tt_shr;
            case lex_detail::pat_eq: return parser_detail::tt_eq;
            case lex_detail::pat_ne: return parser_detail::tt_ne;
            case lex_detail::pat_le: return parser_detail::tt_le;
            case lex_detail::pat_ge: return parser_detail::tt_ge;
            case lex_detail::pat_and: return parser_detail::tt_and;
            case lex_detail::pat_or: return parser_detail::tt_or;
            case lex_detail::pat_add_assign: return parser_detail::tt_add_assign;
            case lex_detail::pat_sub_assign: return parser_detail::tt_sub_assign;
            case lex_detail::pat_mul_assign: return parser_detail::tt_mul_assign;
            case lex_detail::pat_div_assign: return parser_detail::tt_div_assign;
            case lex_detail::pat_mod_assign: return parser_detail::tt_mod_assign;
            case lex_detail::pat_and_assign: return parser_detail::tt_and_assign;
            case lex_detail::pat_or_assign: return parser_detail::tt_or_assign;
            case lex_detail::pat_xor_assign: return parser_detail::tt_xor_assign;
            case lex_detail::pat_shl_assign: return parser_detail::tt_shl_assign;
            case lex_detail::pat_shr_assign: return parser_detail::tt_shr_assign;
            case lex_detail::pat_arrow: return parser_detail::tt_arrow;

            // ------ string literal
            case lex_detail::pat_string: lex_state_stack_.push_back(lex_detail::sc_string); break;
            case lex_detail::pat_string_seq: txt.append(lexeme, llen); break;
            case lex_detail::pat_string_ln_wrap: in_ctx->text.pos.nextLn(); break;  // Skip '\n'
            case lex_detail::pat_string_nl: {
                logger::warning(SymbolLoc(tkn.loc.loc_ctx, tkn.loc.last)).format("line break in string literal");
                txt.push_back('\n');
                in_ctx->text.pos.nextLn();
            } break;
            case lex_detail::pat_string_close: {
                tkn.val = std::move(txt);
                lex_state_stack_.pop_back();
                return parser_detail::tt_string_literal;
            } break;

            // ------ boolean literals
            case lex_detail::pat_true_literal: tkn.val = true; return parser_detail::tt_bool_literal;
            case lex_detail::pat_false_literal: tkn.val = false; return parser_detail::tt_bool_literal;

            // ------ numerical literals
            case lex_detail::pat_dec_literal: {
                tkn.val = IntegerConst::fromString(10, tkn.loc, std::string_view(lexeme, llen));
                return parser_detail::tt_int_literal;
            } break;
            case lex_detail::pat_hex_literal: {
                tkn.val = IntegerConst::fromString(16, tkn.loc, std::string_view(lexeme + 2, llen - 2));
                return parser_detail::tt_int_literal;
            } break;
            case lex_detail::pat_real_literal: {
                tkn.val = uxs::from_string<double>(std::string_view(lexeme, llen));
                return parser_detail::tt_real_literal;
            } break;

            // ------ identifiers
            case lex_detail::pat_id: {
                auto id = std::string_view(lexeme, llen);
                if (auto it = keywords_.find(id); it != keywords_.end()) { return it->second; }
                tkn.val = id;
                return parser_detail::tt_id;
            } break;

            // ------ comments
            case lex_detail::pat_comment1: {  // Eat up comment `// ...`
                skipTillNewLine(in_ctx->text);
                if (in_ctx->text.first != in_ctx->text.last) { ++in_ctx->text.first, in_ctx->text.pos.nextLn(); }
                tkn.loc.first = in_ctx->text.pos;
            } break;
            case lex_detail::pat_comment2: {  // Eat up comment `/* ... */`
                bool is_terminated = skipCommentBlock(in_ctx->text);
                if (!is_terminated) { logger::warning(tkn.loc).format("unterminated comment block"); }
                tkn.loc.first = in_ctx->text.pos;
            } break;

            // ------ other
            case lex_detail::pat_fake_nl:
            case lex_detail::pat_nl: {
                in_ctx->text.pos.nextLn();
                tkn.loc.first = in_ctx->text.pos;
            } break;
            case lex_detail::pat_whitespace: tkn.loc.first.col = in_ctx->text.pos.col; break;
            case lex_detail::pat_ellipsis: return parser_detail::tt_ellipsis;
            case lex_detail::pat_scope_resolution: return parser_detail::tt_scope_resolution;
            case lex_detail::pat_esc_char: return static_cast<unsigned char>(lexeme[1]);
            case lex_detail::pat_concatenate: return parser_detail::tt_concatenate;
            case lex_detail::pat_sharp: {
                if (!!(in_ctx->flags & InputContext::Flags::kPreprocDirective)) { return '#'; }
                parsePreprocessorDirective();
                reset_token_loc(*(in_ctx = &getInputContext()));
            } break;
            case lex_detail::pat_other_char: return static_cast<unsigned char>(lexeme[0]);
            default: return parser_detail::tt_end_of_file;
        }
    }
    return parser_detail::tt_end_of_file;
}

const InputFileInfo* DaisyParserPass::loadInputFile(std::string_view file_path) {
    std::filesystem::path path(file_path);
    if (path.is_relative()) { path = (std::filesystem::current_path() / path).lexically_normal(); }

    std::string normal_path = path.generic_string();
    auto it = ctx_->input_files.find(normal_path);
    if (it != ctx_->input_files.end()) { return &it->second; }

    if (!std::filesystem::exists(path)) { return nullptr; }

    uxs::filebuf ifile(normal_path.c_str(), "r");
    if (!ifile) { return nullptr; }

    auto& file_info = ctx_->input_files
                          .emplace(std::piecewise_construct, std::forward_as_tuple(std::move(normal_path)),
                                   std::forward_as_tuple(ctx_, std::string(file_path)))
                          .first->second;

    size_t file_sz = static_cast<size_t>(ifile.seek(0, uxs::seekdir::kEnd));
    file_info.text = std::make_unique<char[]>(file_sz);
    ifile.seek(0);
    size_t n_read = ifile.read(uxs::as_span(file_info.text.get(), file_sz));

    auto get_next_line = [](const char* text, const char* boundary) {
        return std::string_view(text, std::find_if(text, boundary, [](char ch) { return !ch || ch == '\n'; }) - text);
    };

    const char* boundary = file_info.text.get() + n_read;
    const char* last = file_info.text.get() +
                       file_info.text_lines.emplace_back(get_next_line(file_info.text.get(), boundary)).size();
    while (last != boundary && *last) {
        last += file_info.text_lines.emplace_back(get_next_line(++last, boundary)).size();
    }

    file_info.text_size = last - file_info.text.get();
    return &file_info;
}

void DaisyParserPass::ensureEndOfInput(SymbolInfo& tkn) {
    if (lex(tkn) != parser_detail::tt_end_of_input) {
        logger::warning(tkn.loc).format("extra tokens at end of preprocessor directive");
    }
}

void DaisyParserPass::parsePreprocessorDirective() {
    auto& in_ctx = getInputContext();

    // Save input context state
    TextRange text = in_ctx.text;
    InputContext::Flags flags = in_ctx.flags;

    // Limit input by one line
    skipTillNewLine(text);
    in_ctx.text.last = text.first;
    in_ctx.flags = InputContext::Flags::kPreprocDirective | InputContext::Flags::kStopAtEndOfInput;

    SymbolInfo tkn;
    int tt = lex(tkn);  // Parse directive name
    if (tt == parser_detail::tt_id) {
        auto it = preproc_directive_parsers_.find(std::get<std::string_view>(tkn.val));
        if (it != preproc_directive_parsers_.end()) {
            it->second->func(this, tkn);
        } else {
            logger::error(tkn.loc).format("unknown preprocessor directive");
        }
    } else {
        logger::error(tkn.loc).format("expected preprocessor directive name");
    }

    if (text.first != text.last) { ++text.first, text.pos.nextLn(); }
    in_ctx.text = text, in_ctx.flags = flags;
}

void daisy::logSyntaxError(int tt, const SymbolLoc& loc) {
    std::string_view msg;
    switch (tt) {
        case parser_detail::tt_end_of_file: msg = "unexpected end of file"; break;
        case parser_detail::tt_end_of_input: msg = "expected token in expression"; break;
        default: msg = "unexpected token"; break;
    }
    logger::error(loc).format(msg);
}