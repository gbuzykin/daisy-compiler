#pragma once

#include "ir/nodes/node.h"

namespace daisy {
namespace ir {

class RootNode : public util::rtti_mixin<RootNode, Node> {
 public:
    RootNode() : rtti_mixin_t(std::make_unique<Namespace>(nullptr)) {}
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::RootNode);
