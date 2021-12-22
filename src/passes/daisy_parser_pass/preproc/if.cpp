#include "../daisy_parser_pass.h"
#include "ctx/ctx.h"
#include "logger.h"

using namespace daisy;

namespace {

bool evalCondition(DaisyParserPass* pass, SymbolInfo& tkn) {
    uxs::basic_inline_dynbuffer<int, 1> parser_state_stack;
    std::vector<SymbolInfo> symbol_stack;

    parser_state_stack.reserve_at_curr(256);
    symbol_stack.reserve(128);

    auto& in_ctx = pass->getInputContext();
    in_ctx.flags &= ~InputContext::Flags::kDisableMacroExpansion;

    auto cast_to_bool = [](const auto& tkn) {
        if (std::holds_alternative<bool>(tkn.val)) { return std::get<bool>(tkn.val); }
        return !std::get<IntegerConst>(tkn.val).isZero();
    };

    auto check_integer_type = [](const auto& tkn) {
        if (std::holds_alternative<IntegerConst>(tkn.val)) { return true; }
        logger::error(tkn.loc).format("expected integer expression");
        return false;
    };

    auto check_non_zero = [](const auto& tkn) {
        if (!std::get<IntegerConst>(tkn.val).isZero()) { return true; }
        logger::error(tkn.loc).format("integer division by zero");
        return false;
    };

    auto check_sign_mismatch = [](const auto* rhs) {
        if (std::get<IntegerConst>(rhs[0].val).isSigned() != std::get<IntegerConst>(rhs[2].val).isSigned()) {
            logger::warning(rhs[1].loc).format("signed/unsigned mismatch");
        }
    };

    int tt = pass->lex(tkn);
    parser_state_stack.push_back(parser_detail::sc_preproc_condition);  // Push initial state
    while (true) {
        int* prev_parser_state_stack_top = parser_state_stack.curr();
        parser_state_stack.reserve_at_curr(1);
        int act = DaisyParserPass::parse(tt, parser_state_stack.first(), parser_state_stack.p_curr(), 0);
        if (act < 0) {
            logSyntaxError(tt, tkn.loc);
            return true;
        } else if (act != parser_detail::predef_act_shift) {
            unsigned rlen = static_cast<unsigned>(1 + prev_parser_state_stack_top - parser_state_stack.curr());
            if (rlen == 0) { symbol_stack.emplace_back().loc = tkn.loc, ++rlen; }  // Empty production workaround
            auto* rhs = &*(symbol_stack.end() - rlen);
            if (act >= parser_detail::act_preproc_op_u_minus && act <= parser_detail::act_preproc_op_binary_not) {
                if (!check_integer_type(rhs[1])) { return true; }
                const auto& operand = std::get<IntegerConst>(rhs[1].val);
                switch (act) {
                    case parser_detail::act_preproc_op_u_minus: rhs[0].val = -operand; break;
                    case parser_detail::act_preproc_op_u_plus: rhs[0].val = operand; break;
                    case parser_detail::act_preproc_op_binary_not: rhs[0].val = ~operand; break;
                    default: break;
                }
            } else if (act >= parser_detail::act_preproc_op_add && act <= parser_detail::act_preproc_op_gt) {
                if (!check_integer_type(rhs[0]) || !check_integer_type(rhs[2])) { return true; }
                const auto& operand1 = std::get<IntegerConst>(rhs[0].val);
                const auto& operand2 = std::get<IntegerConst>(rhs[2].val);
                switch (act) {
                    case parser_detail::act_preproc_op_add: rhs[0].val = operand1 + operand2; break;
                    case parser_detail::act_preproc_op_sub: rhs[0].val = operand1 - operand2; break;
                    case parser_detail::act_preproc_op_mul: rhs[0].val = operand1 * operand2; break;
                    case parser_detail::act_preproc_op_div: {
                        if (!check_non_zero(rhs[2])) { return true; }
                        rhs[0].val = operand1 / operand2;
                    } break;
                    case parser_detail::act_preproc_op_mod: {
                        if (!check_non_zero(rhs[2])) { return true; }
                        rhs[0].val = operand1 % operand2;
                    } break;
                    case parser_detail::act_preproc_op_shl: rhs[0].val = operand1 << operand2; break;
                    case parser_detail::act_preproc_op_shr: rhs[0].val = operand1 >> operand2; break;
                    case parser_detail::act_preproc_op_binary_and: rhs[0].val = operand1 & operand2; break;
                    case parser_detail::act_preproc_op_binary_or: rhs[0].val = operand1 | operand2; break;
                    case parser_detail::act_preproc_op_binary_xor: rhs[0].val = operand1 ^ operand2; break;
                    case parser_detail::act_preproc_op_eq: rhs[0].val = operand1 == operand2; break;
                    case parser_detail::act_preproc_op_ne: rhs[0].val = operand1 != operand2; break;
                    case parser_detail::act_preproc_op_lt: {
                        check_sign_mismatch(rhs);
                        rhs[0].val = operand1 < operand2;
                    } break;
                    case parser_detail::act_preproc_op_le: {
                        check_sign_mismatch(rhs);
                        rhs[0].val = operand1 <= operand2;
                    } break;
                    case parser_detail::act_preproc_op_ge: {
                        check_sign_mismatch(rhs);
                        rhs[0].val = operand1 >= operand2;
                    } break;
                    case parser_detail::act_preproc_op_gt: {
                        check_sign_mismatch(rhs);
                        rhs[0].val = operand1 > operand2;
                    } break;
                    default: break;
                }
            } else {
                switch (act) {
                    case parser_detail::act_preproc_op_logical_not: rhs[0].val = !cast_to_bool(rhs[1]); break;
                    case parser_detail::act_preproc_op_logical_and: {
                        rhs[0].val = cast_to_bool(rhs[0]) && cast_to_bool(rhs[2]);
                    } break;
                    case parser_detail::act_preproc_op_logical_or: {
                        rhs[0].val = cast_to_bool(rhs[0]) || cast_to_bool(rhs[2]);
                    } break;
                    case parser_detail::act_preproc_brackets: rhs[0].val = rhs[1].val; break;
                    case parser_detail::act_preproc_id_ref: rhs[0].val = IntegerConst(IntegerType::i32, 0); break;
                    case parser_detail::act_preproc_op_conditional: {
                        rhs[0].val = cast_to_bool(rhs[0]) ? std::get<IntegerConst>(rhs[2].val) :
                                                            std::get<IntegerConst>(rhs[4].val);
                    } break;
                    case parser_detail::act_preproc_operator_begin: {
                        in_ctx.flags |= InputContext::Flags::kDisableMacroExpansion;
                    } break;
                    case parser_detail::act_preproc_operator_end: {
                        in_ctx.flags &= ~InputContext::Flags::kDisableMacroExpansion;
                        const auto& id = std::get<std::string>(rhs[0].val);
                        if (id == "defined") {
                            const auto& macro_id = std::get<std::string>(rhs[3].val);
                            const auto& ctx = pass->getCompilationContext();
                            rhs[0].val = ctx.macro_defs.find(macro_id) != ctx.macro_defs.end();
                        } else {
                            logger::error(rhs[0].loc).format("unknown preprocessor operator");
                            return true;
                        }
                    } break;
                    default: break;
                }
            }

            if (rlen > 1) { (symbol_stack.end() - rlen)->loc += (symbol_stack.end() - 1)->loc; }
            symbol_stack.erase(symbol_stack.end() - rlen + 1, symbol_stack.end());
        } else if (tt != parser_detail::tt_end_of_input) {
            if (std::holds_alternative<std::string_view>(tkn.val)) {
                tkn.val = std::string(std::get<std::string_view>(tkn.val));
            }
            symbol_stack.emplace_back(std::move(tkn));
            tt = pass->lex(tkn);
        } else {
            break;
        }
    }
    assert(symbol_stack.size() == 1);
    return cast_to_bool(symbol_stack.back());
}

bool evalIsDefined(DaisyParserPass* pass, SymbolInfo& tkn) {
    int tt = pass->lex(tkn);
    if (tt != parser_detail::tt_id) {
        logger::error(tkn.loc).format("expected macro name");
        return true;
    }

    const auto macro_id = std::get<std::string_view>(tkn.val);
    const auto& ctx = pass->getCompilationContext();
    bool result = ctx.macro_defs.find(macro_id) != ctx.macro_defs.end();
    pass->ensureEndOfInput(tkn);
    return result;
}

void parseIfDirective(DaisyParserPass* pass, SymbolInfo& tkn,
                      bool (*eval_condition)(DaisyParserPass* pass, SymbolInfo& tkn)) {
    auto* if_section = pass->getIfSection();
    if (if_section && if_section->section_disable_counter > 0) {
        ++if_section->section_disable_counter;
        return;
    }

    if_section = &pass->pushIfSection(tkn.loc);
    if (eval_condition(pass, tkn)) {
        if_section->is_matched = true;
    } else {  // Skip disabled block
        if_section->section_disable_counter = 1;
    }
}

void parseIfDirective(DaisyParserPass* pass, SymbolInfo& tkn) { parseIfDirective(pass, tkn, evalCondition); }
void parseIfdefDirective(DaisyParserPass* pass, SymbolInfo& tkn) { parseIfDirective(pass, tkn, evalIsDefined); }
void parseIfndefDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    parseIfDirective(pass, tkn, [](DaisyParserPass* pass, SymbolInfo& tkn) { return !evalIsDefined(pass, tkn); });
}

void parseElifDirective(DaisyParserPass* pass, SymbolInfo& tkn,
                        bool (*eval_condition)(DaisyParserPass* pass, SymbolInfo& tkn)) {
    auto* if_section = pass->getIfSection();
    if (!if_section) {
        logger::error(tkn.loc).format("`#elif` without `#if`");
        return;
    }

    if (if_section->has_else_section) { logger::error(tkn.loc).format("`#elif` after `#else`"); }

    if (if_section->section_disable_counter > 1) { return; }

    if (!if_section->is_matched && eval_condition(pass, tkn)) {
        if_section->is_matched = true;
        if_section->section_disable_counter = 0;
    } else {  // Skip further if false or one of conditions is already matched
        if_section->section_disable_counter = 1;
    }
}

void parseElifDirective(DaisyParserPass* pass, SymbolInfo& tkn) { parseElifDirective(pass, tkn, evalCondition); }
void parseElifdefDirective(DaisyParserPass* pass, SymbolInfo& tkn) { parseElifDirective(pass, tkn, evalIsDefined); }
void parseElifndefDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    parseElifDirective(pass, tkn, [](DaisyParserPass* pass, SymbolInfo& tkn) { return !evalIsDefined(pass, tkn); });
}

void parseElseDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    auto* if_section = pass->getIfSection();
    if (!if_section) {
        logger::error(tkn.loc).format("`#else` without `#if`");
        return;
    }

    if (if_section->has_else_section) { logger::error(tkn.loc).format("multiple `#else` section"); }

    if (if_section->section_disable_counter > 1) { return; }

    pass->ensureEndOfInput(tkn);
    if (!if_section->is_matched) {
        if_section->is_matched = true;
        if_section->section_disable_counter = 0;
    } else {  // Skip further if one of conditions is already matched
        if_section->section_disable_counter = 1;
    }
    if_section->has_else_section = true;
}

void parseEndifDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    auto* if_section = pass->getIfSection();
    if (!if_section) {
        logger::error(tkn.loc).format("`#endif` without `#if`");
        return;
    }

    pass->ensureEndOfInput(tkn);
    if (if_section->section_disable_counter > 1) {
        --if_section->section_disable_counter;
    } else {
        pass->popIfSection();
    }
}

}  // namespace

DAISY_ADD_PREPROC_DIRECTIVE_PARSER("if", parseIfDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("ifdef", parseIfdefDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("ifndef", parseIfndefDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("elif", parseElifDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("elifdef", parseElifdefDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("elifndef", parseElifndefDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("else", parseElseDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER("endif", parseEndifDirective, true);