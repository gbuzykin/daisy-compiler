#include "../daisy_parser_pass.h"
#include "ir/nodes/const_def_node.h"

using namespace daisy;

namespace {

void defineConstValue(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto const_def_node = std::make_unique<ir::ConstDefNode>(std::string(std::get<std::string_view>(ss[0].val)),
                                                             ss[0].loc);
    const_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[3].val)));
    pass->getIrNode().pushChildBack(std::move(const_def_node));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_const_value, defineConstValue);
