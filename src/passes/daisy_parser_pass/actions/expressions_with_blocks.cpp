#include "../daisy_parser_pass.h"
#include "ir/nodes/discard_expr_node.h"

using namespace daisy;

namespace {

void beginBlockExpr(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    pass->pushIrNode(*ss[-1].val.emplace<std::unique_ptr<ir::Node>>(
        std::make_unique<ir::Node>(std::make_unique<ir::Namespace>(), loc)));
}

void pushExpressionResult(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    pass->getIrNode().pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
}

void discardExpressionResult(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto discard_node = std::make_unique<ir::DiscardExprNode>(ss[0].loc);
    discard_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
    pass->getIrNode().pushChildBack(std::move(discard_node));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_block_expr, beginBlockExpr);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_push_expr_result, pushExpressionResult);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_discard_expr_result, discardExpressionResult);
