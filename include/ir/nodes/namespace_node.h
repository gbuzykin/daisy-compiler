#pragma once

#include "ir/nodes/named_node.h"

namespace daisy {
namespace ir {

class NamespaceNode : public util::rtti_mixin<NamespaceNode, NamedNode> {
 public:
    NamespaceNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), std::make_unique<Namespace>(parent_scope), loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NamespaceNode);
