#pragma once

#include "ir/nodes/named_node.h"
#include "ir/type_descriptor.h"

namespace daisy {
namespace ir {

class DefNode : public util::rtti_mixin<DefNode, NamedNode> {
 public:
    DefNode(std::string name, const SymbolLoc& loc) : rtti_mixin_t(std::move(name), loc) {}

    const TypeDescriptor* getTypeDescriptor() const { return type_desc_.get(); }
    TypeDescriptor* getTypeDescriptor() { return type_desc_.get(); }
    TypeDescriptor& getOrCreateTypeDescriptor(DataTypeClass type_class, TypeDefNode* type_def = nullptr) {
        return type_desc_ ? *type_desc_ : *(type_desc_ = std::make_unique<TypeDescriptor>(type_class, type_def));
    }

 private:
    std::unique_ptr<TypeDescriptor> type_desc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::DefNode);
