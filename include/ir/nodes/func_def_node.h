#pragma once

#include "ir/nodes/def_node.h"

namespace daisy {
namespace ir {

class FuncDefNode : public util::rtti_mixin<FuncDefNode, DefNode> {
 public:
    FuncDefNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), std::make_unique<Namespace>(parent_scope), loc) {}

    const SymbolLoc& getFullDefinitionLoc() const { return full_def_loc_; }
    void setDefinitionLoc(const SymbolLoc& loc) { full_def_loc_ = loc; }
    std::string getProtoString() const;

 private:
    SymbolLoc full_def_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::FuncDefNode);
