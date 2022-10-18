#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class TernaryOpNode : public util::rtti_mixin<TernaryOpNode, EvalNode> {
 public:
    TernaryOpNode(const SymbolLoc& loc, const SymbolLoc& op1_loc, const SymbolLoc& op2_loc)
        : rtti_mixin_t(loc), op1_loc_(op1_loc), op2_loc_(op2_loc) {}

    const SymbolLoc& getOp1Loc() const { return op1_loc_; }
    const SymbolLoc& getOp2Loc() const { return op2_loc_; }

 private:
    SymbolLoc op1_loc_, op2_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::TernaryOpNode);
