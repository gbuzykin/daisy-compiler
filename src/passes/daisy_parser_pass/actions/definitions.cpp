#include "../daisy_parser_pass.h"
#include "ir/nodes/const_def_node.h"
#include "logger.h"

using namespace daisy;

namespace {

void defineConst(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto const_def_node = std::make_unique<ir::ConstDefNode>(std::string(std::get<std::string_view>(ss[0].val)),
                                                             ss[0].loc);
    logger::debug(const_def_node->getLoc()).format("defining constant `{}`", const_def_node->getName());
    const_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[3].val)));
    pass->getIrNode().pushChildBack(std::move(const_def_node));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_const, defineConst);
