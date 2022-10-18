#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

enum class EvalOperator {
    kUnaryMinus = 0,
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
    OpNode(EvalOperator type, const SymbolLoc& loc, const SymbolLoc& op_loc)
        : rtti_mixin_t(loc), type_(type), op_loc_(op_loc) {}

    EvalOperator getType() const { return type_; }
    const SymbolLoc& getOpLoc() const { return op_loc_; }

 private:
    EvalOperator type_;
    SymbolLoc op_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::OpNode);
