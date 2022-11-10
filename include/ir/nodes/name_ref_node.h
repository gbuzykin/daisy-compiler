#pragma once

#include "ir/nodes/eval_node.h"
#include "ir/nodes/named_node.h"
#include "ir/scope_descriptor.h"

namespace daisy {
namespace ir {

class NameRefNode : public util::rtti_mixin<NameRefNode, EvalNode> {
 public:
    NameRefNode(std::string name, ScopeDescriptor scope_desc, const SymbolLoc& loc)
        : rtti_mixin_t(loc), name_(std::move(name)), scope_desc_(std::move(scope_desc)) {}

    const std::string& getName() const { return name_; }
    const NamedNode* getNamedNode() const { return named_node_; }
    NamedNode* getNamedNode() { return named_node_; }

 private:
    std::string name_;
    ScopeDescriptor scope_desc_;
    NamedNode* named_node_ = nullptr;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NameRefNode);
