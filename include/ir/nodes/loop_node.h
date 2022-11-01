#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class LoopNode : public util::rtti_mixin<LoopNode, EvalNode> {
 public:
    LoopNode(const SymbolLoc& loc, const SymbolLoc& loop_loc) : rtti_mixin_t(loc), loop_loc_(loop_loc) {}

    const SymbolLoc& getLoopLoc() const { return loop_loc_; }

 private:
    SymbolLoc loop_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::LoopNode);
