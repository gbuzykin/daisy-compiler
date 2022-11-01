#pragma once

#include "ir/nodes/def_node.h"

namespace daisy {
namespace ir {

class ConstDefNode : public util::rtti_mixin<ConstDefNode, DefNode> {
 public:
    ConstDefNode(std::string name, const SymbolLoc& loc) : rtti_mixin_t(std::move(name), loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::ConstDefNode);
