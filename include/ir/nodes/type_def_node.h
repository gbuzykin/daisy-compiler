#pragma once

#include "ir/nodes/named_node.h"

namespace daisy {
namespace ir {

class TypeDefNode : public util::rtti_mixin<TypeDefNode, NamedNode> {
 public:
    TypeDefNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), std::make_unique<Namespace>(parent_scope), loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::TypeDefNode);
