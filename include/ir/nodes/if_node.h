#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class IfNode : public util::rtti_mixin<IfNode, EvalNode> {
 public:
    IfNode(const SymbolLoc& loc, const SymbolLoc& if_loc, const SymbolLoc& else_loc = {})
        : rtti_mixin_t(loc), if_loc_(if_loc), else_loc_(else_loc) {}

    const SymbolLoc& getIfLoc() const { return if_loc_; }
    const SymbolLoc& getElseLoc() const { return else_loc_; }

 private:
    SymbolLoc if_loc_, else_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::IfNode);
