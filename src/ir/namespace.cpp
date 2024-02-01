#include "ir/namespace.h"

#include "ir/nodes/def_node.h"
#include "ir/nodes/named_scope_node.h"
#include "logger.h"

using namespace daisy;

namespace {

template<typename NamedNodeTy>
std::pair<NamedNodeTy*, bool> defineName(ir::Namespace& nmspace, NamedNodeTy& named_node) {
    auto* existing_named_node = nmspace.findNode<NamedNodeTy>(named_node.getName());
    if (!existing_named_node) { return {&nmspace.addNode(named_node), true}; }
    logger::error(named_node.getLoc()).println("redefinition of `{}`", named_node.getName());
    logger::note(existing_named_node->getLoc()).println("previous definition is here");
    return {existing_named_node, false};
}

}  // namespace

std::pair<ir::NamedScopeNode*, bool> ir::Namespace::defineName(ir::NamedScopeNode& named_scope) {
    return ::defineName(*this, named_scope);
}

std::pair<ir::DefNode*, bool> ir::Namespace::defineName(ir::DefNode& def_node) { return ::defineName(*this, def_node); }
