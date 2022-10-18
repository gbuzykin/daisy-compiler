#pragma once

#include "ir/nodes/eval_node.h"
#include "ir/nodes/named_node.h"

namespace daisy {
namespace ir {

class NameRefNode : public util::rtti_mixin<NameRefNode, EvalNode> {
 public:
    NameRefNode(std::string name, const SymbolLoc& loc) : rtti_mixin_t(loc), name_(std::move(name)) {}

    const NamedNode* getNamedNode() const { return named_node_; }
    NamedNode* getNamedNode() { return named_node_; }
    const Namespace* getSpecifiedNamespace() const { return specified_namespace_; }
    void setSpecifiedNamespace(const Namespace* nmspace) { specified_namespace_ = nmspace; }

 private:
    std::string name_;
    NamedNode* named_node_ = nullptr;
    const Namespace* specified_namespace_ = nullptr;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NameRefNode);
