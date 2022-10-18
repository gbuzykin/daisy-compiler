#pragma once

#include "ir/float_const.h"
#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class FloatConstNode : public util::rtti_mixin<FloatConstNode, EvalNode> {
 public:
    explicit FloatConstNode(FloatConst v, const SymbolLoc& loc) : rtti_mixin_t(loc), val_(v) {}

    FloatConst getValue() const { return val_; }

 private:
    FloatConst val_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::FloatConstNode);
