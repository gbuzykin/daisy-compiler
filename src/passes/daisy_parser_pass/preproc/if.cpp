#include "../daisy_parser_pass.h"
#include "ctx/ctx.h"
#include "logger.h"

using namespace daisy;

namespace {

bool evalCondition(DaisyParserPass* pass, SymbolInfo& tkn) {
    uxs::inline_basic_dynbuffer<int, 1> parser_state_stack;
    std::vector<SymbolInfo> symbol_stack;

    parser_state_stack.reserve(256);
    symbol_stack.reserve(128);

    auto& in_ctx = pass->getInputContext();
    in_ctx.flags &= ~InputContext::Flags::kDisableMacroExpansion;

    auto cast_to_bool = [](const auto& tkn) {
        if (std::holds_alternative<bool>(tkn.val)) { return std::get<bool>(tkn.val); }
        return !std::get<ir::IntConst>(tkn.val).isZero();
    };

    auto check_integer_type = [](const auto& tkn) {
        if (std::holds_alternative<ir::IntConst>(tkn.val)) { return true; }
        logger::error(tkn.loc).println("expected integer expression");
        return false;
    };

    auto check_non_zero = [](const auto& tkn) {
        if (!std::get<ir::IntConst>(tkn.val).isZero()) { return true; }
        logger::error(tkn.loc).println("integer division by zero");
        return false;
    };

    auto check_sign_mismatch = [](const auto* ss) {
        if (std::get<ir::IntConst>(ss[0].val).isSigned() != std::get<ir::IntConst>(ss[2].val).isSigned()) {
            logger::warning(ss[1].loc).println("signed/unsigned mismatch");
        }
    };

    int tt = pass->lex(tkn);
    parser_state_stack.push_back(parser_detail::sc_preproc_condition);  // Push initial state
    while (true) {
        int* prev_parser_state_stack_top = parser_state_stack.curr();
        parser_state_stack.reserve(1);
        int act = DaisyParserPass::parse(tt, parser_state_stack.data(), parser_state_stack.p_curr(), 0);
        if (act < 0) {
            logSyntaxError(tt, tkn.loc);
            return true;
        } else if (act != parser_detail::predef_act_shift) {
            unsigned rlen = static_cast<unsigned>(1 + prev_parser_state_stack_top - parser_state_stack.curr());
            if (rlen == 0) { symbol_stack.emplace_back().loc = tkn.loc, ++rlen; }  // Empty production workaround
            auto* ss = &*(symbol_stack.end() - rlen);
            if (act == parser_detail::act_preproc_op_u_minus || act == parser_detail::act_preproc_op_u_plus ||
                act == parser_detail::act_preproc_op_bitwise_not) {
                if (!check_integer_type(ss[1])) { return true; }
                const auto& operand = std::get<ir::IntConst>(ss[1].val);
                switch (act) {
                    case parser_detail::act_preproc_op_u_minus: ss[0].val = -operand; break;
                    case parser_detail::act_preproc_op_u_plus: ss[0].val = operand; break;
                    case parser_detail::act_preproc_op_bitwise_not: ss[0].val = ~operand; break;
                    default: break;
                }
            } else if (act >= parser_detail::act_preproc_op_add && act <= parser_detail::act_preproc_op_gt) {
                if (!check_integer_type(ss[0]) || !check_integer_type(ss[2])) { return true; }
                const auto& operand1 = std::get<ir::IntConst>(ss[0].val);
                const auto& operand2 = std::get<ir::IntConst>(ss[2].val);
                switch (act) {
                    case parser_detail::act_preproc_op_add: ss[0].val = operand1 + operand2; break;
                    case parser_detail::act_preproc_op_sub: ss[0].val = operand1 - operand2; break;
                    case parser_detail::act_preproc_op_mul: ss[0].val = operand1 * operand2; break;
                    case parser_detail::act_preproc_op_div: {
                        if (!check_non_zero(ss[2])) { return true; }
                        ss[0].val = operand1 / operand2;
                    } break;
                    case parser_detail::act_preproc_op_mod: {
                        if (!check_non_zero(ss[2])) { return true; }
                        ss[0].val = operand1 % operand2;
                    } break;
                    case parser_detail::act_preproc_op_shl: ss[0].val = operand1 << operand2; break;
                    case parser_detail::act_preproc_op_shr: ss[0].val = operand1 >> operand2; break;
                    case parser_detail::act_preproc_op_bitwise_and: ss[0].val = operand1 & operand2; break;
                    case parser_detail::act_preproc_op_bitwise_or: ss[0].val = operand1 | operand2; break;
                    case parser_detail::act_preproc_op_bitwise_xor: ss[0].val = operand1 ^ operand2; break;
                    case parser_detail::act_preproc_op_eq: ss[0].val = operand1 == operand2; break;
                    case parser_detail::act_preproc_op_ne: ss[0].val = operand1 != operand2; break;
                    case parser_detail::act_preproc_op_lt: {
                        check_sign_mismatch(ss);
                        ss[0].val = operand1 < operand2;
                    } break;
                    case parser_detail::act_preproc_op_le: {
                        check_sign_mismatch(ss);
                        ss[0].val = operand1 <= operand2;
                    } break;
                    case parser_detail::act_preproc_op_ge: {
                        check_sign_mismatch(ss);
                        ss[0].val = operand1 >= operand2;
                    } break;
                    case parser_detail::act_preproc_op_gt: {
                        check_sign_mismatch(ss);
                        ss[0].val = operand1 > operand2;
                    } break;
                    default: break;
                }
            } else {
                switch (act) {
                    case parser_detail::act_preproc_op_logical_not: ss[0].val = !cast_to_bool(ss[1]); break;
                    case parser_detail::act_preproc_op_logical_and: {
                        ss[0].val = cast_to_bool(ss[0]) && cast_to_bool(ss[2]);
                    } break;
                    case parser_detail::act_preproc_op_logical_or: {
                        ss[0].val = cast_to_bool(ss[0]) || cast_to_bool(ss[2]);
                    } break;
                    case parser_detail::act_preproc_brackets: ss[0].val = std::move(ss[1].val); break;
                    case parser_detail::act_preproc_op_conditional: {
                        ss[0].val = cast_to_bool(ss[0]) ? std::get<ir::IntConst>(ss[2].val) :
                                                          std::get<ir::IntConst>(ss[4].val);
                    } break;
                    case parser_detail::act_preproc_operator_begin: {
                        in_ctx.flags |= InputContext::Flags::kDisableMacroExpansion;
                    } break;
                    case parser_detail::act_preproc_operator_end: {
                        in_ctx.flags &= ~InputContext::Flags::kDisableMacroExpansion;
                        const auto id = std::get<std::string_view>(ss[0].val);
                        if (id == "defined") {
                            const auto macro_id = std::get<std::string_view>(ss[3].val);
                            const auto& ctx = pass->getCompilationContext();
                            ss[0].val = ctx.macro_defs.find(macro_id) != ctx.macro_defs.end();
                        } else {
                            logger::error(ss[0].loc).println("unknown preprocessing operator");
                            return true;
                        }
                    } break;
                    default: break;
                }
            }

            if (rlen > 1) { (symbol_stack.end() - rlen)->loc += (symbol_stack.end() - 1)->loc; }
            symbol_stack.erase(symbol_stack.end() - rlen + 1, symbol_stack.end());
        } else if (tt != parser_detail::tt_end_of_input) {
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
        logger::error(tkn.loc).println("expected macro identifier");
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

void parseElifDirective(DaisyParserPass* pass, SymbolInfo& tkn,
                        bool (*eval_condition)(DaisyParserPass* pass, SymbolInfo& tkn)) {
    auto* if_section = pass->getIfSection();
    if (!if_section) {
        logger::error(tkn.loc).println("`#elif` without `#if`");
        return;
    }

    if (if_section->has_else_section) { logger::error(tkn.loc).println("`#elif` after `#else`"); }

    if (if_section->section_disable_counter > 1) { return; }

    if (!if_section->is_matched && eval_condition(pass, tkn)) {
        if_section->is_matched = true;
        if_section->section_disable_counter = 0;
    } else {  // Skip further if false or one of conditions is already matched
        if_section->section_disable_counter = 1;
    }
}

void parseElseDirective(DaisyParserPass* pass, SymbolInfo& tkn) {
    auto* if_section = pass->getIfSection();
    if (!if_section) {
        logger::error(tkn.loc).println("`#else` without `#if`");
        return;
    }

    if (if_section->has_else_section) { logger::error(tkn.loc).println("multiple `#else` section"); }

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
        logger::error(tkn.loc).println("`#endif` without `#if`");
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

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(
    if, [](DaisyParserPass* pass, SymbolInfo& tkn) { parseIfDirective(pass, tkn, evalCondition); }, true);

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(
    ifdef, [](DaisyParserPass* pass, SymbolInfo& tkn) { parseIfDirective(pass, tkn, evalIsDefined); }, true);

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(
    ifndef,
    [](DaisyParserPass* pass, SymbolInfo& tkn) {
        parseIfDirective(pass, tkn, [](DaisyParserPass* pass, SymbolInfo& tkn) { return !evalIsDefined(pass, tkn); });
    },
    true);

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(
    elif, [](DaisyParserPass* pass, SymbolInfo& tkn) { parseElifDirective(pass, tkn, evalCondition); }, true);

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(
    elifdef, [](DaisyParserPass* pass, SymbolInfo& tkn) { parseElifDirective(pass, tkn, evalIsDefined); }, true);

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(
    elifndef,
    [](DaisyParserPass* pass, SymbolInfo& tkn) {
        parseElifDirective(pass, tkn, [](DaisyParserPass* pass, SymbolInfo& tkn) { return !evalIsDefined(pass, tkn); });
    },
    true);

DAISY_ADD_PREPROC_DIRECTIVE_PARSER(else, parseElseDirective, true);
DAISY_ADD_PREPROC_DIRECTIVE_PARSER(endif, parseEndifDirective, true);
