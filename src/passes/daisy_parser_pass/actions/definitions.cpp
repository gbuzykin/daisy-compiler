#include "../daisy_parser_pass.h"
#include "ir/nodes/const_def_node.h"
#include "ir/nodes/func_def_node.h"
#include "ir/nodes/var_def_node.h"
#include "logger.h"

using namespace daisy;

namespace {

void defineConst(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto const_def_node = std::make_unique<ir::ConstDefNode>(std::string(std::get<std::string_view>(ss[0].val)),
                                                             ss[0].loc);
    logger::debug(const_def_node->getLoc()).format("defining constant `{}`", const_def_node->getName());
    const_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[3].val)));
    pass->getCurrentScope().pushChildBack(std::move(const_def_node));
}

void defineVariable(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto var_def_node = std::make_unique<ir::VarDefNode>(std::string(std::get<std::string_view>(ss[0].val)), ss[0].loc);
    logger::debug(var_def_node->getLoc()).format("defining variable `{}`", var_def_node->getName());
    var_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[3].val)));
    pass->getCurrentScope().pushChildBack(std::move(var_def_node));
}

void beginFuncProto(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto func_def_node = std::make_unique<ir::FuncDefNode>(std::string(std::get<std::string_view>(ss[-2].val)),
                                                           pass->getCurrentScope(), ss[-2].loc);
    logger::debug(func_def_node->getLoc()).format("defining function `{}`", func_def_node->getName());
    pass->setCurrentScope(*ss[-2].val.emplace<std::unique_ptr<ir::Node>>(std::move(func_def_node)));
}

void addFuncFormalArg(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    pass->getCurrentScope().pushChildBack(
        std::make_unique<ir::DefNode>(std::string(std::get<std::string_view>(ss[0].val)), ss[0].loc));
}

void endFuncDecl(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto func_def_node = std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val));
    logger::debug(loc).format("function `{}` declaration", util::cast<ir::NamedNode&>(*func_def_node).getName());
    pass->popCurrentScope();
    pass->getCurrentScope().pushChildBack(std::move(func_def_node));
}

void endFuncDef(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto func_def_node = std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val));
    logger::debug(ss[0].loc + ss[1].loc)
        .format("function `{}` definition", util::cast<ir::NamedNode&>(*func_def_node).getName());
    func_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    pass->popCurrentScope();
    pass->getCurrentScope().pushChildBack(std::move(func_def_node));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_const, defineConst);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_variable, defineVariable);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_func_proto, beginFuncProto);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_add_func_formal_arg, addFuncFormalArg);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_func_decl, endFuncDecl);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_func_def, endFuncDef);
