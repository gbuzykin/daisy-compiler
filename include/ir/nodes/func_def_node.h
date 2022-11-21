#pragma once

#include "ir/nodes/def_node.h"

namespace daisy {
namespace ir {

class FuncDefNode : public util::rtti_mixin<FuncDefNode, DefNode> {
 public:
    FuncDefNode(std::string name, Node& parent_scope, const SymbolLoc& loc)
        : rtti_mixin_t(std::move(name), std::make_unique<Namespace>(parent_scope), loc) {}

    bool isDefined() const { return is_defined_; }
    const SymbolLoc& getDefinitionLoc() const { return def_loc_; }
    void setDefined(const SymbolLoc& loc) { is_defined_ = true, def_loc_ = loc; }
    std::string getProtoString() const;

 private:
    bool is_defined_ = false;
    SymbolLoc def_loc_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::FuncDefNode);
