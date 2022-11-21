#include "ir/nodes/named_node.h"

#include "ir/nodes/named_scope_node.h"
#include "ir/nodes/root_node.h"

using namespace daisy;

RTTI_IMPLEMENT_TYPE_INFO(ir::NamedNode);

std::string ir::NamedNode::getGlobalName() const {
    const auto* parent = getParent();
    if (!util::is_kind_of<NamedScopeNode>(parent)) { return name_; }

    std::vector<const NamedScopeNode*> path;
    path.reserve(16);
    path.push_back(static_cast<const NamedScopeNode*>(parent));
    while (util::is_kind_of<NamedScopeNode>((parent = parent->getNamespace().getParentScope()))) {
        path.push_back(static_cast<const NamedScopeNode*>(parent));
    }
    if (!util::is_kind_of<RootNode>(parent)) { return name_; }

    std::string global_name("::");
    for (const auto& scope : uxs::make_reverse_range(path)) {
        global_name += scope->getName();
        global_name += "::";
    }
    global_name += name_;
    return global_name;
}
