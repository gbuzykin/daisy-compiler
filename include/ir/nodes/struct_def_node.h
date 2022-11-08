#pragma once

#include "ir/nodes/type_def_node.h"

namespace daisy {
namespace ir {

class StructDefNode : public util::rtti_mixin<StructDefNode, TypeDefNode> {
 public:
    StructDefNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), parent_scope, loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::StructDefNode);
