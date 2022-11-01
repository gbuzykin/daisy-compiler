#include "../daisy_parser_pass.h"
#include "ir/nodes/bool_const_node.h"
#include "ir/nodes/float_const_node.h"
#include "ir/nodes/if_node.h"
#include "ir/nodes/int_const_node.h"
#include "ir/nodes/name_ref_node.h"
#include "ir/nodes/op_node.h"
#include "ir/nodes/string_const_node.h"

using namespace daisy;

namespace {

void makeUnaryMinusNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kUnaryMinus, loc, ss[0].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeUnaryPlusNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kUnaryPlus, loc, ss[0].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeAddNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kAdd, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeSubNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kSub, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeMulNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kMul, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeDivNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kDiv, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeModNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kMod, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeShlNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kShl, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeShrNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kShr, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeBitwiseNotNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kBitwiseNot, loc, ss[0].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeBitwiseAndNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kBitwiseAnd, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeBitwiseOrNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kBitwiseOr, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeBitwiseXorNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kBitwiseXor, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeEqNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kEq, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeNeNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kNe, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeLtNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kLt, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeLeNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kLe, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeGeNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kGe, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeGtNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kGt, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeLogicalNotNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kLogicalNot, loc, ss[0].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeLogicalAndNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kLogicalAnd, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeLogicalOrNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(ir::EvalOperator::kLogicalOr, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeConditionalNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::IfNode>(loc, ss[1].loc, ss[3].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(node));
}

void makeBracketsNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) { ss[0] = std::move(ss[1]); }

void makeNameRefNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(
        std::make_unique<ir::NameRefNode>(std::move(std::get<std::string>(ss[0].val)), loc));
}

void makeBoolConstLiteralNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::make_unique<ir::BoolConstNode>(std::get<bool>(ss[0].val), loc));
}

void makeIntConstLiteralNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(
        std::make_unique<ir::IntConstNode>(std::get<ir::IntConst>(ss[0].val), loc));
}

void makeFloatConstLiteralNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(
        std::make_unique<ir::FloatConstNode>(std::get<ir::FloatConst>(ss[0].val), loc));
}

void makeStringConstLiteralNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(
        std::make_unique<ir::StringConstNode>(std::move(std::get<std::string>(ss[0].val)), loc));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_u_minus, makeUnaryMinusNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_u_plus, makeUnaryPlusNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_add, makeAddNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_sub, makeSubNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_mul, makeMulNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_div, makeDivNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_mod, makeModNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_shl, makeShlNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_shr, makeShrNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_bitwise_not, makeBitwiseNotNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_bitwise_and, makeBitwiseAndNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_bitwise_or, makeBitwiseOrNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_bitwise_xor, makeBitwiseXorNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_eq, makeEqNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_ne, makeNeNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_lt, makeLtNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_le, makeLeNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_ge, makeGeNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_gt, makeGtNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_logical_not, makeLogicalNotNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_logical_and, makeLogicalAndNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_logical_or, makeLogicalOrNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_op_conditional, makeConditionalNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_brackets, makeBracketsNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_expr_name_ref, makeNameRefNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_bool_const_literal, makeBoolConstLiteralNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_int_const_literal, makeIntConstLiteralNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_float_const_literal, makeFloatConstLiteralNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_string_const_literal, makeStringConstLiteralNode);
