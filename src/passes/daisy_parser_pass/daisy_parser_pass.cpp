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

namespace {
std::unordered_map<std::string_view, int> g_keywords = {
    {"namespace", parser_detail::tt_namespace},
    {"const", parser_detail::tt_const},
    {"let", parser_detail::tt_let},
    {"func", parser_detail::tt_func},
    {"if", parser_detail::tt_if},
    {"else", parser_detail::tt_else},
    {"loop", parser_detail::tt_loop},
    {"while", parser_detail::tt_while},
    {"struct", parser_detail::tt_struct},
    {"mut", parser_detail::tt_mut},
};
}

void DaisyParserPass::configure() {
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
    if_section_stack_.clear();
}

PassResult DaisyParserPass::run(CompilationContext& ctx) {
    uxs::inline_basic_dynbuffer<int, 1> parser_state_stack;
    std::vector<SymbolInfo> symbol_stack;

    ctx_ = &ctx;
    error_status_ = 0;
    lex_state_stack_.reserve(256);
    parser_state_stack.reserve(1024);
    symbol_stack.reserve(1024);

    ctx_->ir_root = std::make_unique<ir::RootNode>();
    current_scope_ = ctx_->ir_root.get();

    defineBuiltinMacros();

    // Create main source file input context
    const auto* src_file = pushInputFile(ctx.file_name, {});
    if (!src_file) {
        logger::fatal().println("could not open input file `{}`", ctx.file_name);
        return PassResult::kFatalError;
    }

    lex_state_stack_.push_back(lex_detail::sc_initial);

    // Parse input file
    int tt = lex(la_tkn_);
    parser_state_stack.push_back(parser_detail::sc_initial);  // Push initial state
    while (true) {
        int* prev_parser_state_stack_top = parser_state_stack.curr();
        parser_state_stack.reserve(1);
        int act = parse(tt, parser_state_stack.data(), parser_state_stack.p_curr(), 0);
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
            SymbolLoc loc = (symbol_stack.end() - rlen)->loc;
            if (rlen > 1) { loc += (symbol_stack.end() - 1)->loc; }  // Default reduction location
            if (act > parser_detail::predef_act_reduce && reduce_action_handlers_[act]) {
                reduce_action_handlers_[act](this, &*(symbol_stack.end() - rlen), loc);
            }
            (symbol_stack.end() - rlen)->loc = loc;
            symbol_stack.erase(symbol_stack.end() - rlen + 1, symbol_stack.end());
        } else if (tt != parser_detail::tt_end_of_file) {
            if (logger::g_debug_level >= 3) {
                if (tt == parser_detail::tt_id) {
                    logger::debug(la_tkn_.loc, true).println("id: {}", std::get<std::string_view>(la_tkn_.val));
                } else if (tt == parser_detail::tt_string_literal) {
                    logger::debug(la_tkn_.loc, true)
                        .println("string: {}", uxs::make_quoted_string(std::get<std::string>(la_tkn_.val)));
                } else if (tt == parser_detail::tt_int_literal) {
                    if (std::get<ir::IntConst>(la_tkn_.val).isSigned()) {
                        logger::debug(la_tkn_.loc, true)
                            .println("integer number: {}", std::get<ir::IntConst>(la_tkn_.val).getValue<int64_t>());
                    } else {
                        logger::debug(la_tkn_.loc, true)
                            .println("integer number: {}", std::get<ir::IntConst>(la_tkn_.val).getValue<uint64_t>());
                    }
                } else if (tt == parser_detail::tt_float_literal) {
                    logger::debug(la_tkn_.loc, true)
                        .println("float number: {}", std::get<ir::FloatConst>(la_tkn_.val).getValue<double>());
                } else {
                    logger::debug(la_tkn_.loc, true).println("token");
                }
            }
            symbol_stack.emplace_back(std::move(la_tkn_));
            tt = lex(la_tkn_);
            if (error_status_) { --error_status_; }
        } else {
            break;
        }
    }

    assert(ctx.error_count || !current_scope_->getParent());
    return ctx.error_count == 0 ? PassResult::kSuccess : PassResult::kError;
}

int DaisyParserPass::lex(SymbolInfo& tkn, bool* leading_ws) {
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
            int lex_flags = at_beginning_of_line_;
            const char* last = in_ctx->text.last;
            if (lex_state_stack_.avail() < static_cast<size_t>(last - first)) {
                last = first + lex_state_stack_.avail();
                lex_flags |= lex_detail::flag_has_more;
            }
            assert(first <= last);
            pat = lex_detail::lex(first, last, lex_state_stack_.p_curr(), &llen, lex_flags);
            if (pat >= lex_detail::predef_pat_default) {
                break;
            } else if (lex_flags & lex_detail::flag_has_more) {
                // enlarge state stack and continue analysis
                lex_state_stack_.reserve(llen);
                first = last;
            } else {  // Input buffer is over
                if (lex_state_stack_.back() == lex_detail::sc_string) {
                    logger::warning(tkn.loc).println("unterminated string literal");
                    tkn.val = std::move(txt);
                    lex_state_stack_.back() = lex_detail::sc_initial;
                    return parser_detail::tt_string_literal;
                }
                tkn.loc.last = tkn.loc.first = in_ctx->text.pos;
                if (!!(in_ctx->flags & InputContext::Flags::kStopAtEndOfInput)) {
                    return parser_detail::tt_end_of_input;
                }
                while (in_ctx->last_if_section_state != getIfSection()) {  // Remove unclosed if sections
                    logger::warning(if_section_stack_.front().loc).println("`#if` without `#endif`");
                    popIfSection();
                }
                // Input context stack is empty - end of compilation unit
                if (popInputContext()) { return parser_detail::tt_end_of_file; }
                reset_token_loc(*(in_ctx = &getInputContext()));
                // Proceed to the next token
                first = lexeme = in_ctx->text.first;
            }
        }

        at_beginning_of_line_ = 0;
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
                logger::warning(tkn.loc).println("unknown escape sequence");
                txt.push_back(lexeme[1]);
            } break;
            case lex_detail::pat_escape_hex: {
                char escape = uxs::dig_v(lexeme[2]);
                if (llen > 3) { escape = (escape << 4) + uxs::dig_v(lexeme[3]); }
                txt.push_back(escape);
            } break;
            case lex_detail::pat_escape_oct: {
                char escape = uxs::dig_v(lexeme[1]);
                if (llen > 2) { escape = (escape << 3) + uxs::dig_v(lexeme[2]); }
                if (llen > 3) { escape = (escape << 3) + uxs::dig_v(lexeme[3]); }
                txt.push_back(escape);
            } break;

            // ------ operators
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
            case lex_detail::pat_string: lex_state_stack_.back() = lex_detail::sc_string; break;
            case lex_detail::pat_string_seq: txt.append(lexeme, llen); break;
            case lex_detail::pat_string_ln_wrap: in_ctx->text.pos.nextLn(); break;  // Skip '\n'
            case lex_detail::pat_string_nl: {
                logger::warning(SymbolLoc(tkn.loc.loc_ctx, tkn.loc.last)).println("line break in string literal");
                txt.push_back('\n');
                in_ctx->text.pos.nextLn();
            } break;
            case lex_detail::pat_string_close: {
                tkn.val = std::move(txt);
                lex_state_stack_.back() = lex_detail::sc_initial;
                return parser_detail::tt_string_literal;
            } break;

            // ------ boolean literals
            case lex_detail::pat_true_literal: tkn.val = true; return parser_detail::tt_bool_literal;
            case lex_detail::pat_false_literal: tkn.val = false; return parser_detail::tt_bool_literal;

            // ------ numerical literals
            case lex_detail::pat_bin_literal: {
                tkn.val = ir::IntConst::fromString(2, tkn.loc, std::string_view(lexeme + 2, llen - 2));
                return parser_detail::tt_int_literal;
            } break;
            case lex_detail::pat_oct_literal: {
                tkn.val = ir::IntConst::fromString(8, tkn.loc, std::string_view(lexeme, llen));
                return parser_detail::tt_int_literal;
            } break;
            case lex_detail::pat_dec_literal: {
                tkn.val = ir::IntConst::fromString(10, tkn.loc, std::string_view(lexeme, llen));
                return parser_detail::tt_int_literal;
            } break;
            case lex_detail::pat_hex_literal: {
                tkn.val = ir::IntConst::fromString(16, tkn.loc, std::string_view(lexeme + 2, llen - 2));
                return parser_detail::tt_int_literal;
            } break;
            case lex_detail::pat_float_literal: {
                tkn.val = ir::FloatConst::fromString(tkn.loc, std::string_view(lexeme, llen));
                return parser_detail::tt_float_literal;
            } break;

            // ------ identifiers
            case lex_detail::pat_id: {
                auto id = std::string_view(lexeme, llen);
                if (const auto* macro_exp = in_ctx->macro_expansion) {
                    assert(macro_exp->macro_def);
                    const auto& macro_def = *macro_exp->macro_def;
                    if (auto it = macro_def.formal_args.find(id); it != macro_def.formal_args.end()) {
                        expandMacroArgument(macro_exp->actual_args[it->second.first]);
                        reset_token_loc(*(in_ctx = &getInputContext()));
                        break;
                    }
                }
                if (!(in_ctx->flags & InputContext::Flags::kDisableMacroExpansion)) {
                    if (auto it = ctx_->macro_defs.find(id); it != ctx_->macro_defs.end()) {
                        expandMacro(tkn.loc, *it->second);
                        reset_token_loc(*(in_ctx = &getInputContext()));
                        break;
                    }
                }
                if (!(in_ctx->flags & InputContext::Flags::kPreprocDirective)) {
                    if (auto it = g_keywords.find(id); it != g_keywords.end()) { return it->second; }
                }
                tkn.val = id;
                return parser_detail::tt_id;
            } break;

            // ------ comments
            case lex_detail::pat_comment1: {  // Eat up comment `// ...`
                skipTillNewLine(in_ctx->text);
                if (in_ctx->text.first != in_ctx->text.last) { ++in_ctx->text.first, in_ctx->text.pos.nextLn(); }
                tkn.loc.first = in_ctx->text.pos;
                if (!(in_ctx->flags & InputContext::Flags::kExpendingMacro)) {
                    at_beginning_of_line_ = lex_detail::flag_at_beg_of_line;
                }
                if (leading_ws) { *leading_ws = true; }
            } break;
            case lex_detail::pat_comment2: {  // Eat up comment `/* ... */`
                bool is_terminated = skipCommentBlock(in_ctx->text);
                if (!is_terminated) { logger::warning(tkn.loc).println("unterminated comment block"); }
                tkn.loc.first = in_ctx->text.pos;
                if (leading_ws) { *leading_ws = true; }
            } break;

            // ------ other
            case lex_detail::pat_nl: {
                if (!(in_ctx->flags & InputContext::Flags::kExpendingMacro)) {
                    at_beginning_of_line_ = lex_detail::flag_at_beg_of_line;
                }
            } /* fallthrough */
            case lex_detail::pat_fake_nl: {
                in_ctx->text.pos.nextLn();
                tkn.loc.first = in_ctx->text.pos;
                if (leading_ws) { *leading_ws = true; }
            } break;
            case lex_detail::pat_whitespace: {
                tkn.loc.first.col = in_ctx->text.pos.col;
                if (leading_ws) { *leading_ws = true; }
            } break;
            case lex_detail::pat_ellipsis: return parser_detail::tt_ellipsis;
            case lex_detail::pat_scope_resolution: return parser_detail::tt_scope_resolution;
            case lex_detail::pat_esc_char: return static_cast<unsigned char>(lexeme[1]);
            case lex_detail::pat_concatenate: return parser_detail::tt_concatenate;
            case lex_detail::pat_sharp: {
                parsePreprocessorDirective();
                reset_token_loc(*(in_ctx = &getInputContext()));
            } break;
            case lex_detail::pat_other_char: return static_cast<unsigned char>(lexeme[0]);
            default: return parser_detail::tt_end_of_file;
        }
    }
    return parser_detail::tt_end_of_file;
}

/*static*/ int DaisyParserPass::parse(int tt, int* sptr0, int** p_sptr, int rise_error) {
    return parser_detail::parse(tt, sptr0, p_sptr, rise_error);
}

const InputFileInfo* DaisyParserPass::pushInputFile(std::string_view file_path, const SymbolLoc& expansion_loc) {
    std::filesystem::path path(file_path);
    if (path.is_relative()) { path = (std::filesystem::current_path() / path).lexically_normal(); }

    std::string normal_path = path.generic_string();
    auto it = ctx_->input_files.find(normal_path);
    InputFileInfo* file_info = it != ctx_->input_files.end() ? it->second.get() : nullptr;

    if (!file_info) {
        if (!std::filesystem::exists(path)) { return nullptr; }

        uxs::filebuf ifile(normal_path.c_str(), "r");
        if (!ifile) { return nullptr; }

        file_info = ctx_->input_files
                        .emplace(std::move(normal_path), std::make_unique<InputFileInfo>(ctx_, std::string(file_path)))
                        .first->second.get();

        size_t file_sz = static_cast<size_t>(ifile.seek(0, uxs::seekdir::end));
        file_info->text = std::make_unique<char[]>(file_sz);
        ifile.seek(0);
        size_t n_read = ifile.read(uxs::as_span(file_info->text.get(), file_sz));

        auto get_next_line = [](const char* text, const char* boundary) {
            return std::string_view(text,
                                    std::find_if(text, boundary, [](char ch) { return !ch || ch == '\n'; }) - text);
        };

        const char* boundary = file_info->text.get() + n_read;
        const char* last = file_info->text.get() +
                           file_info->text_lines.emplace_back(get_next_line(file_info->text.get(), boundary)).size();
        while (last != boundary && *last) {
            last += file_info->text_lines.emplace_back(get_next_line(++last, boundary)).size();
        }

        file_info->text_size = last - file_info->text.get();
    }

    pushInputContext(
        std::make_unique<InputContext>(file_info->getText(), &newLocationContext(file_info, expansion_loc)));
    at_beginning_of_line_ = lex_detail::flag_at_beg_of_line;
    return file_info;
}

bool DaisyParserPass::isKeyword(std::string_view id) const { return g_keywords.find(id) != g_keywords.end(); }

void DaisyParserPass::ensureEndOfInput(SymbolInfo& tkn) {
    if (lex(tkn) != parser_detail::tt_end_of_input) {
        logger::warning(tkn.loc).println("extra tokens at end of preprocessing directive");
    }
}

void DaisyParserPass::parsePreprocessorDirective() {
    auto& in_ctx = getInputContext();
    bool is_text_disabled = false;

    do {
        // Save input context state
        TextRange text = in_ctx.text;
        InputContext::Flags flags = in_ctx.flags;

        // Limit input by one line
        skipTillNewLine(text);
        in_ctx.text.last = text.first;
        in_ctx.flags = InputContext::Flags::kPreprocDirective | InputContext::Flags::kStopAtEndOfInput |
                       InputContext::Flags::kDisableMacroExpansion;

        SymbolInfo tkn;
        int tt = lex(tkn);  // Parse directive name
        if (tt == parser_detail::tt_id) {
            auto it = preproc_directive_parsers_.find(std::get<std::string_view>(tkn.val));
            if (it != preproc_directive_parsers_.end()) {
                if (!is_text_disabled || it->second->parse_disabled_text) { it->second->func(this, tkn); }
            } else if (!is_text_disabled) {
                logger::error(tkn.loc).println("unknown preprocessing directive");
            }
        } else if (!is_text_disabled) {
            logger::error(tkn.loc).println("expected preprocessing directive identifier");
        }

        if (!!(in_ctx.flags & InputContext::Flags::kSkipFile)) { text.first = text.last; }
        in_ctx.text = text, in_ctx.flags = flags;

        const auto* if_section = getIfSection();
        if (!if_section || if_section->section_disable_counter == 0) { break; }

        // Eat up all text till the position after single `#` symbol
        // Note: strings and comments are skipped
        skipTillPreprocDirective(in_ctx.text);
        is_text_disabled = true;
    } while (in_ctx.text.first != in_ctx.text.last);
}

void daisy::logSyntaxError(int tt, const SymbolLoc& loc) {
    std::string_view msg;
    switch (tt) {
        case parser_detail::tt_end_of_file: msg = "unexpected end of file"; break;
        case parser_detail::tt_end_of_input: msg = "expected token in expression"; break;
        default: msg = "unexpected token"; break;
    }
    logger::error(loc).println(uxs::make_runtime_string(msg));
}
