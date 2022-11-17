#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

enum class EvalOperator {
    kAssign = 0,
    kUnaryMinus,
    kUnaryPlus,
    kAdd,
    kSub,
    kMul,
    kDiv,
    kMod,
    kShl,
    kShr,
    kBitwiseNot,
    kBitwiseAnd,
    kBitwiseOr,
    kBitwiseXor,
    kEq,
    kNe,
    kLt,
    kLe,
    kGe,
    kGt,
    kLogicalNot,
    kLogicalAnd,
    kLogicalOr,
};

class OpNode : public util::rtti_mixin<OpNode, EvalNode> {
 public:
    OpNode(EvalOperator op, const SymbolLoc& loc, const SymbolLoc& op_loc)
        : rtti_mixin_t(loc), op_(op), op_loc_(op_loc) {}

    EvalOperator getOp() const { return op_; }
    const SymbolLoc& getOpLoc() const { return op_loc_; }

 private:
    EvalOperator op_;
    SymbolLoc op_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::OpNode);
