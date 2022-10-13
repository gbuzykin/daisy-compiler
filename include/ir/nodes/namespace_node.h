#pragma once

#include "ir/nodes/named_node.h"

namespace daisy {
namespace ir {

class NamespaceNode : public util::rtti_mixin<NamespaceNode, NamedNode> {
 public:
    explicit NamespaceNode(std::string name) : rtti_mixin_t(std::move(name), std::make_unique<Namespace>()) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NamespaceNode);
