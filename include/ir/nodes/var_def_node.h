#pragma once

#include "ir/nodes/named_node.h"

namespace daisy {
namespace ir {

class VarDefNode : public util::rtti_mixin<VarDefNode, NamedNode> {
 public:
    VarDefNode(std::string name, const SymbolLoc& loc) : rtti_mixin_t(std::move(name), loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::VarDefNode);
