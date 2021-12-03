#include "../daisy_parser_pass.h"
#include "../text_utils.h"
#include "ctx/ctx.h"
#include "logger.h"
#include "uxs/algorithm.h"

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
    MacroDefinition macro_def{MacroDefinition::Type::kUserDefined, id, false, tkn.loc};

    auto& in_ctx = pass->getInputContext();
    if (in_ctx.text.first != in_ctx.text.last && *in_ctx.text.first == '(') {  // Parse argument list
        unsigned count = 0;
        ++in_ctx.text.first, ++in_ctx.text.pos.col;
        while (true) {
            std::string_view arg_id;
            if (tt = pass->lex(tkn); tt == parser_detail::tt_id) {
                arg_id = std::get<std::string_view>(tkn.val);
                if (arg_id == kVaArgsId) {  // variable argument identifier
                    logger::error(tkn.loc).format("identifier `{}` is reserved for variable argument", arg_id);
                    return;
                }
            } else if (tt == parser_detail::tt_ellipsis) {
                arg_id = kVaArgsId, macro_def.is_variadic = true;
            } else {
                logger::error(tkn.loc).format("expected macro argument identifier or `...`");
                return;
            }
            macro_def.formal_args.emplace(arg_id, std::make_pair(count++, tkn.loc));
            if ((tt = pass->lex(tkn)) == parser_detail::tt_ellipsis) {
                if (macro_def.is_variadic) {
                    logger::error(tkn.loc).format("expected `,`");
                    return;
                }
                macro_def.is_variadic = true;
                tt = pass->lex(tkn);
            }
            if (tt == ')') {
                break;
            } else if (tt != ',') {
                logger::error(tkn.loc).format("expected `)` or `,`");
                return;
            } else if (macro_def.is_variadic) {
                logger::error(tkn.loc).format("expected `)`");
                return;
            }
        }
    }

    macro_def.text = in_ctx.text;
    skipWhitespaces(macro_def.text);
    macro_def.text.last = trimTrailingWhitespaces(macro_def.text.first, macro_def.text.last);

    auto& ctx = pass->getCompilationContext();
    if (!ctx.macro_defs.insert_or_assign(id, std::move(macro_def)).second) {
        if (macro_def.type != MacroDefinition::Type::kUserDefined) {
            logger::warning(tkn.loc).format("builtin macro `{}` redefinition", id);
        } else {
            logger::warning(tkn.loc).format("macro `{}` redefinition", id);
        }
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
        if (it->second.type != MacroDefinition::Type::kUserDefined) {
            logger::warning(tkn.loc).format("cannot undefine builtin macro `{}`", id);
        }
        ctx.macro_defs.erase(id);
    } else {
        logger::warning(tkn.loc).format("macro `{}` is not defined", id);
    }
    pass->ensureEndOfInput(tkn);
}

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

void DaisyParserPass::expandMacro(const SymbolLoc& loc, const MacroDefinition& macro_def) {
    auto& in_ctx = getInputContext();
    const std::string_view id = macro_def.id;
    MacroExpansion macro_exp{&macro_def, &in_ctx, loc};

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

    if (checkMacroExpansionForRecursion(id)) {
        logger::error(loc).format("recursive macro `{}` expansion", id);
        return;
    }

    // Push expansion text input context
    auto macro_exp_ctx = std::make_unique<MacroExpansionContext>(
        macro_def.text,
        &newLocationContext(macro_def.loc.loc_ctx ? macro_def.loc.loc_ctx->file : nullptr,
                            TextExpansion{macro_exp.loc, &macro_def}),
        std::move(macro_exp));
    macro_exp_ctx->macro_expansion = &macro_exp_ctx->macro_expansion_info;
    pushInputContext(std::move(macro_exp_ctx));
}

void DaisyParserPass::expandMacroArgument(const TextRange& arg) {
    // Push macro argument input context
    // Note: use location context of macro expansion source
    const auto* source_in_ctx = getInputContext().macro_expansion->source_in_ctx;
    assert(source_in_ctx);
    pushInputContext(std::make_unique<InputContext>(arg, source_in_ctx->loc_ctx)).macro_expansion =
        source_in_ctx->macro_expansion;
}
