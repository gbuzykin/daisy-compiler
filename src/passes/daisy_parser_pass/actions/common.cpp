#include "../daisy_parser_pass.h"
#include "ir/nodes/namespace_node.h"
#include "logger.h"

using namespace daisy;

namespace {

void concatenateStringConst(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    std::get<std::string>(ss[0].val) += std::get<std::string>(ss[1].val);
}

void setLocalScope(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<ir::ScopeDescriptor>(ir::ScopeClass::kLocal, pass->getCurrentScope());
}

void setRootScope(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<ir::ScopeDescriptor>(ir::ScopeClass::kSpecified, pass->getRootScope());
}

void resolveScope(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto name = std::get<std::string_view>(ss[1].val);
    auto& scope_desc = std::get<ir::ScopeDescriptor>(ss[0].val);
    if (scope_desc.getClass() != ir::ScopeClass::kInvalid) {
        if (auto* scope = scope_desc.lookupName<ir::NamedScopeNode>(name)) {
            ss[0].val.emplace<ir::ScopeDescriptor>(ir::ScopeClass::kSpecified, *scope);
            return;
        } else {
            logger::error(ss[1].loc).format("undefined namespace identifier `{}`", name);
        }
    }
    ss[0].val.emplace<ir::ScopeDescriptor>(ir::ScopeClass::kInvalid);
}

void beginNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[-2].val);
    auto& nmspace = pass->getCurrentScope().getNamespace();
    auto* nmspace_node = nmspace.findNode<ir::NamedScopeNode>(name);
    if (!util::is_kind_of<ir::NamespaceNode>(nmspace_node)) {
        auto& new_nmspace_node = pass->getCurrentScope().pushChildBack(
            std::make_unique<ir::NamespaceNode>(std::string(name), pass->getCurrentScope(), ss[-2].loc));
        if (!nmspace_node) {
            nmspace.addNode(new_nmspace_node);
        } else {
            logger::error(ss[-3].loc + ss[-2].loc).format("typename `{}` redefinition", name);
        }
        nmspace_node = &new_nmspace_node;
    } else {
        logger::debug(ss[-1].loc).format("entering existing namespace `{}`", name);
    }
    pass->setCurrentScope(*nmspace_node);
}

void endBlock(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) { pass->popCurrentScope(); }

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_concatenate_string_const, concatenateStringConst);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_local_scope, setLocalScope);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_root_scope, setRootScope);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_resolve_scope, resolveScope);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_namespace, beginNamespace);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_block, endBlock);
