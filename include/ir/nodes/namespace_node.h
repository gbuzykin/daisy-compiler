#pragma once

#include "ir/nodes/named_scope_node.h"

namespace daisy {
namespace ir {

class NamespaceNode : public util::rtti_mixin<NamespaceNode, NamedScopeNode> {
 public:
    NamespaceNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), parent_scope, loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NamespaceNode);
