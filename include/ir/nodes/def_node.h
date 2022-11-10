#pragma once

#include "ir/nodes/named_node.h"
#include "ir/type_descriptor.h"

namespace daisy {
namespace ir {

class DefNode : public util::rtti_mixin<DefNode, NamedNode> {
 public:
    DefNode(std::string name, const SymbolLoc& loc) : rtti_mixin_t(std::move(name), loc) {}
    DefNode(std::string name, std::unique_ptr<Namespace> nmspace, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), std::move(nmspace), loc) {}

    const TypeDescriptor& getTypeDescriptor() const { return type_desc_; }
    TypeDescriptor& getTypeDescriptor() { return type_desc_; }

 private:
    TypeDescriptor type_desc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::DefNode);
