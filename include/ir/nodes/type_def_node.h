#pragma once

#include "ir/nodes/named_scope_node.h"

namespace daisy {
namespace ir {

class TypeDefNode : public util::rtti_mixin<TypeDefNode, NamedScopeNode> {
 public:
    TypeDefNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), parent_scope, loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::TypeDefNode);
