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

void makeUnaryOpNode(ir::EvalOperator op, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(op, loc, ss[0].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val = std::move(node);
}

void makeBinaryOpNode(ir::EvalOperator op, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::OpNode>(op, loc, ss[1].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val = std::move(node);
}

}  // namespace

// General operators
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_u_minus, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeUnaryOpNode(ir::EvalOperator::kUnaryMinus, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_u_plus, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeUnaryOpNode(ir::EvalOperator::kUnaryPlus, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_add, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kAdd, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_sub, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kSub, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_mul, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kMul, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_div, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kDiv, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_mod, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kMod, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_shl, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kShl, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_shr, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kShr, ss, loc);
});

// Bitwise operators
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_bitwise_not, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeUnaryOpNode(ir::EvalOperator::kBitwiseNot, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_bitwise_and, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kBitwiseAnd, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_bitwise_or, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kBitwiseOr, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_bitwise_xor, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kBitwiseXor, ss, loc);
});

// Comparison operators
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_eq, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kEq, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_ne, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kNe, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_lt, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kLt, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_le, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kLe, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_ge, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kGe, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_gt, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kGt, ss, loc);
});

// Logical operators
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_logical_not, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeUnaryOpNode(ir::EvalOperator::kLogicalNot, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_logical_and, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kLogicalAnd, ss, loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_logical_or, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kLogicalOr, ss, loc);
});

// Conditional expression
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_op_conditional, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto node = std::make_unique<ir::IfNode>(loc, ss[1].loc, ss[3].loc);
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    ss[0].val = std::move(node);
});

// Brackets
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_brackets, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0] = std::move(ss[1]);
});

// Name reference
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_name_ref, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::make_unique<ir::NameRefNode>(std::string(std::get<std::string_view>(ss[1].val)),
                                                  std::move(std::get<ir::ScopeDescriptor>(ss[0].val)), loc);
});

// Assignment
DAISY_ADD_REDUCE_ACTION_HANDLER(act_expr_assignment, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    makeBinaryOpNode(ir::EvalOperator::kAssign, ss, loc);
});

// Constants
DAISY_ADD_REDUCE_ACTION_HANDLER(act_bool_const_literal, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::make_unique<ir::BoolConstNode>(std::get<bool>(ss[0].val), loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_int_const_literal, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::make_unique<ir::IntConstNode>(std::get<ir::IntConst>(ss[0].val), loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_float_const_literal, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::make_unique<ir::FloatConstNode>(std::get<ir::FloatConst>(ss[0].val), loc);
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_string_const_literal, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::make_unique<ir::StringConstNode>(std::move(std::get<std::string>(ss[0].val)), loc);
});
