#include "../daisy_parser_pass.h"
#include "ir/nodes/namespace_node.h"
#include "logger.h"

using namespace daisy;

namespace {

void beginNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[-2].val);
    auto& nmspace = pass->getIrNode().findNamespace();
    auto* nmspace_node = nmspace.findNode<ir::NamespaceNode>(name);
    if (!nmspace_node) {
        nmspace_node = &pass->getIrNode().pushChildBack(std::make_unique<ir::NamespaceNode>(std::string(name)));
        nmspace.addNode(*nmspace_node);
    } else {
        logger::debug(ss[-1].loc).format("entering existing namespace `{}`", name);
    }
    pass->setIrNode(*nmspace_node);
}

void endNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) { pass->popIrNode(); }

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_namespace, beginNamespace);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_namespace, endNamespace);
