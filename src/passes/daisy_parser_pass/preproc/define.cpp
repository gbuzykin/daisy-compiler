#include "../daisy_parser_pass.h"
#include "../text_utils.h"
#include "ctx/ctx.h"
#include "logger.h"
#include "uxs/algorithm.h"
#include "uxs/stringalg.h"

using namespace daisy;

constexpr std::string_view kVaArgsId = "__va_args__";

namespace {

void parseDefineDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);  // Parse identifier
    if (tt != parser_detail::tt_id) {
        logger::error(tkn.loc).format("expected macro name");
        return;
    }

    const auto id = std::get<std::string_view>(tkn.val);
    if (pass->isKeyword(id)) {
        logger::error(tkn.loc).format("keyword `{}` cannot be used as macro identifier", id);
        return;
    }

    auto macro_def = std::make_unique<MacroDefinition>(MacroDefinition::Type::kUserDefined, id);
    macro_def->loc = tkn.loc;

    auto& in_ctx = pass->getInputContext();
    if (in_ctx.text.first != in_ctx.text.last && *in_ctx.text.first == '(') {  // Parse argument list
        unsigned count = 0;
        ++in_ctx.text.first, ++in_ctx.text.pos.col;
        while (true) {
            std::string_view arg_id;
            if (tt = pass->lex(tkn); tt == parser_detail::tt_id) {
                arg_id = std::get<std::string_view>(tkn.val);
                if (pass->isKeyword(arg_id)) {  // is a keyword
                    logger::error(tkn.loc).format("keyword `{}` cannot be used as macro argument identifier", arg_id);
                    return;
                } else if (arg_id == kVaArgsId) {  // variable argument identifier
                    logger::error(tkn.loc).format("identifier `{}` is reserved for variable argument", arg_id);
                    return;
                }
            } else if (tt == parser_detail::tt_ellipsis) {
                arg_id = kVaArgsId, macro_def->is_variadic = true;
            } else {
                logger::error(tkn.loc).format("expected macro argument identifier or `...`");
                return;
            }
            macro_def->formal_args.emplace(arg_id, std::make_pair(count++, tkn.loc));
            if ((tt = pass->lex(tkn)) == parser_detail::tt_ellipsis) {
                if (macro_def->is_variadic) {
                    logger::error(tkn.loc).format("expected `,`");
                    return;
                }
                macro_def->is_variadic = true;
                tt = pass->lex(tkn);
            }
            if (tt == ')') {
                break;
            } else if (tt != ',') {
                logger::error(tkn.loc).format("expected `)` or `,`");
                return;
            } else if (macro_def->is_variadic) {
                logger::error(tkn.loc).format("expected `)`");
                return;
            }
        }
    }

    macro_def->text = in_ctx.text;
    skipWhitespaces(macro_def->text);
    macro_def->text.last = trimTrailingWhitespaces(macro_def->text.first, macro_def->text.last);

    auto& ctx = pass->getCompilationContext();
    if (auto [it, success] = ctx.macro_defs.try_emplace(id, std::move(macro_def)); !success) {
        if (it->second->type != MacroDefinition::Type::kUserDefined) {
            logger::warning(tkn.loc).format("builtin macro `{}` redefinition", id);
        } else {
            logger::warning(tkn.loc).format("macro `{}` redefinition", id);
        }
        it->second = std::move(macro_def);
    }
}

void parseUndefDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);  // Parse identifier
    if (tt != parser_detail::tt_id) {
        logger::error(tkn.loc).format("expected macro name");
        return;
    }
    auto& ctx = pass->getCompilationContext();
    const auto id = std::get<std::string_view>(tkn.val);
    auto it = ctx.macro_defs.find(id);
    if (it != ctx.macro_defs.end()) {
        if (it->second->type != MacroDefinition::Type::kUserDefined) {
            logger::warning(tkn.loc).format("cannot undefine builtin macro `{}`", id);
        }
        ctx.macro_defs.erase(id);
    } else {
        logger::warning(tkn.loc).format("macro `{}` is not defined", id);
    }
    pass->ensureEndOfInput(tkn);
}

const SymbolLoc* findMacroExpansionOrigin(const SymbolLoc* loc) {
    while (loc->loc_ctx && loc->loc_ctx->expansion.macro_def) { loc = &loc->loc_ctx->expansion.loc; }
    return loc;
}

bool builtinMacroLine(DaisyParserPass* pass, const MacroExpansion& macro_exp) {
    const auto* loc = findMacroExpansionOrigin(&macro_exp.loc);
    pass->pushStringInputContext(uxs::to_string(loc->first.ln), macro_exp);
    return true;
}

bool builtinMacroFile(DaisyParserPass* pass, const MacroExpansion& macro_exp) {
    const auto* loc = findMacroExpansionOrigin(&macro_exp.loc);
    pass->pushStringInputContext(uxs::make_quoted_text(loc->loc_ctx->file ? loc->loc_ctx->file->file_name : ""),
                                 macro_exp);
    return true;
}

bool builtinMacroVaOpt(DaisyParserPass* pass, const MacroExpansion& macro_exp) {
    const auto& in_ctx = pass->getInputContext();
    if (const auto* parent_exp = in_ctx.macro_expansion) {
        assert(parent_exp->macro_def);
        if (parent_exp->macro_def->is_variadic) {
            const auto& va_arg = parent_exp->actual_args.back();
            if (va_arg.first != va_arg.last) {
                // Parent's variable argument is not trivial - expand VaOpt's argument
                pass->pushInputContext(std::make_unique<InputContext>(macro_exp.actual_args.back(), in_ctx.loc_ctx))
                    .macro_expansion = in_ctx.macro_expansion;
            }
            return true;
        }
    }
    return false;
}

std::string stringizeMacro(DaisyParserPass* pass, const MacroExpansion& macro_exp) {
    const auto* loc_ctx = pass->getInputContext().loc_ctx;
    assert(loc_ctx);

    auto& in_ctx = pass->pushInputContext(
        std::make_unique<InputContext>(macro_exp.actual_args.back(), loc_ctx, InputContext::Flags::kStopAtEndOfInput));
    in_ctx.macro_expansion = in_ctx.macro_expansion;

    SymbolInfo tkn;
    bool remove_ws = false;
    std::string text;
    text.reserve(256);
    while (true) {
        bool leading_ws = false;
        int tt = pass->lex(tkn, &leading_ws);
        if (tt == parser_detail::tt_concatenate) {
            remove_ws = true;
        } else if (tt != parser_detail::tt_end_of_input) {
            if (!remove_ws && leading_ws) { text.push_back(' '); }
            remove_ws = false;
            if (tt == parser_detail::tt_string_literal) {
                uxs::make_quoted_text(static_cast<std::string_view>(std::get<std::string>(tkn.val)),
                                      std::back_inserter(text));
            } else {
                const auto& curr_ctx = pass->getInputContext();
                assert(tkn.loc.first.ln == tkn.loc.last.ln && tkn.loc.first.col <= tkn.loc.last.col);
                unsigned tkn_length = tkn.loc.last.col - tkn.loc.first.col + 1;
                std::copy(curr_ctx.text.first - tkn_length, curr_ctx.text.first, std::back_inserter(text));
            }
        } else {
            break;
        }
    }

    pass->popInputContext();
    return text;
}

bool builtinMacroStringize(DaisyParserPass* pass, const MacroExpansion& macro_exp) {
    pass->pushStringInputContext('\"' + stringizeMacro(pass, macro_exp) + '\"', macro_exp);
    return true;
}

bool builtinMacroPaste(DaisyParserPass* pass, const MacroExpansion& macro_exp) {
    pass->pushStringInputContext(stringizeMacro(pass, macro_exp), macro_exp);
    return true;
}

using BuiltInMacroImpl = bool (*)(DaisyParserPass*, const MacroExpansion& macro_exp);
const std::vector<std::tuple<std::string_view, bool, BuiltInMacroImpl>> g_builtin_macro_impl = {
    // <id, is_variadic, impl_func>
    {"__line__", false, builtinMacroLine},   {"__file__", false, builtinMacroFile},
    {"__va_opt__", true, builtinMacroVaOpt}, {"__str__", true, builtinMacroStringize},
    {"__paste__", true, builtinMacroPaste},
};

}  // namespace

DAISY_ADD_PREPROC_DIRECTIVE_PARSER("define", parseDefineDirective);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("undef", parseUndefDirective);

bool DaisyParserPass::checkMacroExpansionForRecursion(std::string_view macro_id) {
    const auto& in_ctx = getInputContext();
    for (const auto* loc_ctx = in_ctx.loc_ctx; loc_ctx->expansion.loc.loc_ctx;
         loc_ctx = loc_ctx->expansion.loc.loc_ctx) {
        if (const auto* macro_def = loc_ctx->expansion.macro_def; macro_def) {
            if (macro_def->id == macro_id) { return true; }
        } else {
            break;
        }
    }
    return false;
}

void DaisyParserPass::defineBuiltinMacros() {
    for (unsigned n = 0; n < g_builtin_macro_impl.size(); ++n) {
        const auto& [id, is_variadic, impl_func] = g_builtin_macro_impl[n];
        ctx_->macro_defs[id] = std::make_unique<MacroDefinition>(MacroDefinition::Type::kBuiltIn + n, id, is_variadic);
    }
}

void DaisyParserPass::expandMacro(const SymbolLoc& loc, const MacroDefinition& macro_def) {
    auto& in_ctx = getInputContext();
    const std::string_view id = macro_def.id;

    auto& loc_ctx = newLocationContext(macro_def.loc.loc_ctx ? macro_def.loc.loc_ctx->file : nullptr, loc, &macro_def);
    auto macro_exp_ctx = std::make_unique<MacroExpansionContext>(macro_def.text, &loc_ctx, &macro_def, &in_ctx, loc);

    auto& macro_exp = macro_exp_ctx->macro_expansion_info;
    macro_exp_ctx->macro_expansion = &macro_exp;

    auto macro_details = [id, &macro_def]() { logger::note(macro_def.loc).format("macro `{}` defined here", id); };

    const size_t formal_arg_count = macro_def.formal_args.empty() && macro_def.is_variadic ?
                                        1 :  // builtin variadic macros workaround
                                        macro_def.formal_args.size();
    if (formal_arg_count) {
        TextRange text = in_ctx.text;
        findMacroArgumentList(text);
        if (text.first == text.last || *text.first != '(') {
            logger::error(loc).format("macro `{}` requires arguments", id);
            macro_details();
            return;
        }

        // Read actual macro arguments
        auto& args = macro_exp.actual_args;
        args.reserve(4);

        while (text.first != text.last && *text.first != ')') {
            ++text.first, ++text.pos.col;  // Set text position next to '(' or ','
            skipWhitespaces(text);
            TextRange arg_text = text;
            findMacroArgumentSeparator(text);
            arg_text.last = trimTrailingWhitespaces(arg_text.first, text.first);
            if (!macro_def.is_variadic || args.size() < formal_arg_count) {
                args.emplace_back(arg_text);
            } else {
                args.back().last = arg_text.last;
            }
        }

        if (text.first == text.last) {
            logger::error(loc).format("unterminated argument list for macro `{}`", id);
            macro_details();
            return;
        }

        // Expand loc till closing bracket
        macro_exp.loc.last = text.pos;
        loc_ctx.expansion.loc.last = text.pos;

        // Set text position next to ')'
        ++text.first, ++text.pos.col;
        in_ctx.text = text;

        if (macro_def.is_variadic) {
            if (args.size() < formal_arg_count - 1) {
                logger::error(loc).format("variadic macro `{}` requires at least {} arguments, but {} provided", id,
                                          formal_arg_count - 1, args.size());
                macro_details();
                return;
            }
            if (args.size() < formal_arg_count) { args.emplace_back(TextRange{text.first, text.first, text.pos}); }
        } else if (args.size() != formal_arg_count) {
            logger::error(loc).format("macro `{}` requires {} arguments, but {} provided", id, formal_arg_count,
                                      args.size());
            macro_details();
            return;
        }
    }

    if (macro_def.type >= MacroDefinition::Type::kBuiltIn) {
        auto impl_func = std::get<BuiltInMacroImpl>(
            g_builtin_macro_impl[macro_def.type - MacroDefinition::Type::kBuiltIn]);
        if (!impl_func(this, macro_exp)) { logger::error(loc).format("macro `{}` cannot be used in this context", id); }
        return;
    }

    if (checkMacroExpansionForRecursion(id)) {
        logger::error(loc).format("recursive macro `{}` expansion", id);
        return;
    }

    // Push expansion text input context
    pushInputContext(std::move(macro_exp_ctx));
}

void DaisyParserPass::expandMacroArgument(const TextRange& arg) {
    // Push macro argument input context
    // Note: use location context of macro expansion source
    const auto* source_in_ctx = getInputContext().macro_expansion->source_in_ctx;
    assert(source_in_ctx);
    auto arg_exp_ctx = std::make_unique<InputContext>(arg, source_in_ctx->loc_ctx, InputContext::Flags::kExpendingMacro);
    arg_exp_ctx->macro_expansion = source_in_ctx->macro_expansion;
    pushInputContext(std::move(arg_exp_ctx));
}
