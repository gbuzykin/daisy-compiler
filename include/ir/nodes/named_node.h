#pragma once

#include "ir/nodes/node.h"

#include <string>

namespace daisy {
namespace ir {

class NamedNode : public util::rtti_mixin<NamedNode, Node> {
 public:
    NamedNode(std::string name, const SymbolLoc& loc) : rtti_mixin_t(loc), name_(std::move(name)) {}
    NamedNode(std::string name, std::unique_ptr<Namespace> nmspace, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(nmspace), loc), name_(std::move(name)) {}

    const std::string& getName() const { return name_; }

 private:
    std::string name_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NamedNode);
