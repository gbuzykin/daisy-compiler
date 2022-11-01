#include "../daisy_parser_pass.h"
#include "ir/nodes/discard_expr_node.h"
#include "ir/nodes/if_node.h"
#include "ir/nodes/loop_node.h"

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

void makeIfNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto if_node = std::make_unique<ir::IfNode>(loc, ss[0].loc);
    if_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    if_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(if_node));
}

void makeIfElseNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto if_node = std::make_unique<ir::IfNode>(loc, ss[0].loc, ss[3].loc);
    if_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    if_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    if_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(if_node));
}

void makeEndlessLoopNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto loop_node = std::make_unique<ir::LoopNode>(loc, ss[0].loc);
    loop_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(loop_node));
}

void makeLoopNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto loop_node = std::make_unique<ir::LoopNode>(loc, ss[0].loc);
    loop_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    loop_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(loop_node));
}

void makeLoopWithElseNode(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto loop_node = std::make_unique<ir::LoopNode>(loc, ss[0].loc);
    loop_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    loop_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    loop_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    ss[0].val.emplace<std::unique_ptr<ir::Node>>(std::move(loop_node));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_block_expr, beginBlockExpr);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_push_expr_result, pushExpressionResult);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_discard_expr_result, discardExpressionResult);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_if_expr, makeIfNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_if_else_expr, makeIfElseNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_endless_loop_expr, makeEndlessLoopNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_loop_expr, makeLoopNode);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_loop_with_else_expr, makeLoopWithElseNode);
