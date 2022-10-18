#include "../daisy_parser_pass.h"
#include "ir/nodes/namespace_node.h"
#include "ir/nodes/type_def_node.h"
#include "logger.h"

using namespace daisy;

namespace {

void concatenateStringConst(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    std::get<std::string>(ss[0].val) += std::get<std::string>(ss[1].val);
}

void relativeName(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::string(std::get<std::string_view>(ss[0].val));
}

void absoluteName(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    std::string name("::");
    name += std::get<std::string_view>(ss[1].val);
    ss[0].val = std::move(name);
}

void concatenateNamePath(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& name = std::get<std::string>(ss[0].val);
    name += "::";
    name += std::get<std::string_view>(ss[2].val);
}

void beginNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[-2].val);
    auto& nmspace = pass->getIrNode().findNamespace();
    auto* nmspace_node = nmspace.findNode(
        name, [](auto& node) { return util::is_kind_of<ir::NamespaceNode, ir::TypeDefNode>(node); });
    if (!nmspace_node) {
        nmspace_node = &pass->getIrNode().pushChildBack(std::make_unique<ir::NamespaceNode>(std::string(name)));
        nmspace.addNode(*nmspace_node);
    } else if (!util::is_kind_of<ir::NamespaceNode>(*nmspace_node)) {
        logger::error(ss[-3].loc + ss[-2].loc).format("typename `{}` redefinition", name);
    } else {
        logger::debug(ss[-1].loc).format("entering existing namespace `{}`", name);
    }
    pass->setIrNode(*nmspace_node);
}

void endNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) { pass->popIrNode(); }

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_concatenate_string_const, concatenateStringConst);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_relative_name, relativeName);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_absolute_name, absoluteName);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_concatenate_name_path, concatenateNamePath);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_namespace, beginNamespace);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_namespace, endNamespace);
