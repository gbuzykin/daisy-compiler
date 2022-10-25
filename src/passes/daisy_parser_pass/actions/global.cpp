#include "../daisy_parser_pass.h"
#include "ir/namespace.h"

using namespace daisy;

namespace {

void beginNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[-1].val);
    pass->setCurrentNamespace(pass->getCurrentNamespace().findOrAddObject<ir::Namespace>(std::string(name)));
}

void endNamespace(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto* parent_namespace = pass->getCurrentNamespace().getNamespace();
    assert(parent_namespace);
    pass->setCurrentNamespace(*parent_namespace);
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_namespace, beginNamespace);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_namespace, endNamespace);
