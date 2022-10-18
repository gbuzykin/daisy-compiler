#pragma once

#include "ir/int_const.h"
#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class IntConstNode : public util::rtti_mixin<IntConstNode, EvalNode> {
 public:
    explicit IntConstNode(IntConst v, const SymbolLoc& loc) : rtti_mixin_t(loc), val_(v) {}

    IntConst getValue() const { return val_; }

 private:
    IntConst val_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::IntConstNode);
