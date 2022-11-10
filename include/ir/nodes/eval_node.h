#pragma once

#include "ir/nodes/node.h"
#include "ir/type_descriptor.h"

namespace daisy {
namespace ir {

class EvalNode : public util::rtti_mixin<EvalNode, Node> {
 public:
    explicit EvalNode(const SymbolLoc& loc) : rtti_mixin_t(loc) {}

    const TypeDescriptor& getTypeDescriptor() const { return type_desc_; }
    TypeDescriptor& getTypeDescriptor() { return type_desc_; }

 private:
    TypeDescriptor type_desc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::EvalNode);
