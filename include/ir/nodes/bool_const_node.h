#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class BoolConstNode : public util::rtti_mixin<BoolConstNode, EvalNode> {
 public:
    explicit BoolConstNode(bool v, const SymbolLoc& loc) : rtti_mixin_t(loc), val_(v) {}

    bool getValue() const { return val_; }

 private:
    bool val_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::BoolConstNode);
