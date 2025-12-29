#include "../daisy_parser_pass.h"
#include "ir/nodes/discard_expr_node.h"
#include "ir/nodes/if_node.h"
#include "ir/nodes/loop_node.h"

using namespace daisy;

DAISY_ADD_REDUCE_ACTION_HANDLER(act_begin_block_expr, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    pass->setCurrentScope(*ss[-1].val.emplace<std::unique_ptr<ir::Node>>(
        std::make_unique<ir::Node>(std::make_unique<ir::Namespace>(pass->getCurrentScope()), loc)));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_push_expr_result, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& /*loc*/) {
    pass->getCurrentScope().push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_discard_expr_result, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& /*loc*/) {
    auto& discard_node = pass->getCurrentScope().push_back(std::make_unique<ir::DiscardExprNode>(ss[0].loc));
    discard_node.push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[0].val)));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_if_expr, [](DaisyParserPass* /*pass*/, SymbolInfo* ss, SymbolLoc& loc) {
    auto if_node = std::make_unique<ir::IfNode>(loc, ss[0].loc);
    if_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    if_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val = std::move(if_node);
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_if_else_expr, [](DaisyParserPass* /*pass*/, SymbolInfo* ss, SymbolLoc& loc) {
    auto if_node = std::make_unique<ir::IfNode>(loc, ss[0].loc, ss[3].loc);
    if_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    if_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    if_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    ss[0].val = std::move(if_node);
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_endless_loop_expr, [](DaisyParserPass* /*pass*/, SymbolInfo* ss, SymbolLoc& loc) {
    auto loop_node = std::make_unique<ir::LoopNode>(loc, ss[0].loc);
    loop_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    ss[0].val = std::move(loop_node);
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_loop_expr, [](DaisyParserPass* /*pass*/, SymbolInfo* ss, SymbolLoc& loc) {
    auto loop_node = std::make_unique<ir::LoopNode>(loc, ss[0].loc);
    loop_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    loop_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    ss[0].val = std::move(loop_node);
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_loop_with_else_expr, [](DaisyParserPass* /*pass*/, SymbolInfo* ss, SymbolLoc& loc) {
    auto loop_node = std::make_unique<ir::LoopNode>(loc, ss[0].loc);
    loop_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val)));
    loop_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    loop_node->push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    ss[0].val = std::move(loop_node);
});
