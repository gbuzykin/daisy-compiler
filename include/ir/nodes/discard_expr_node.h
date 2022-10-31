#pragma once

#include "ir/nodes/node.h"

namespace daisy {
namespace ir {

class DiscardExprNode : public util::rtti_mixin<DiscardExprNode, Node> {
 public:
    explicit DiscardExprNode(const SymbolLoc& loc) : rtti_mixin_t(loc) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::DiscardExprNode);
