#pragma once

#include "ir/nodes/eval_node.h"

namespace daisy {
namespace ir {

class StringConstNode : public util::rtti_mixin<StringConstNode, EvalNode> {
 public:
    explicit StringConstNode(std::string v, const SymbolLoc& loc) : rtti_mixin_t(loc), val_(std::move(v)) {}

    const std::string& getValue() const { return val_; }

 private:
    std::string val_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::StringConstNode);
