#include "../daisy_parser_pass.h"
#include "ir/nodes/const_def_node.h"
#include "ir/nodes/func_def_node.h"
#include "ir/nodes/type_def_node.h"
#include "ir/nodes/var_def_node.h"
#include "logger.h"

using namespace daisy;

namespace {

std::unordered_map<std::string_view, ir::DataTypeClass> g_built_in_types = {
    {"bool", ir::DataTypeClass::kBool},       {"i8", ir::DataTypeClass::kInt8},
    {"u8", ir::DataTypeClass::kUInt8},        {"i16", ir::DataTypeClass::kInt16},
    {"u16", ir::DataTypeClass::kUInt16},      {"i32", ir::DataTypeClass::kInt32},
    {"u32", ir::DataTypeClass::kUInt32},      {"int", ir::DataTypeClass::kInt32},
    {"unsigned", ir::DataTypeClass::kUInt32}, {"i64", ir::DataTypeClass::kInt64},
    {"u64", ir::DataTypeClass::kUInt64},      {"f32", ir::DataTypeClass::kFloat32},
    {"float", ir::DataTypeClass::kFloat32},   {"f64", ir::DataTypeClass::kFloat64},
    {"double", ir::DataTypeClass::kFloat64},
};

void defineConst(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto const_def_node = std::make_unique<ir::ConstDefNode>(std::string(std::get<std::string_view>(ss[0].val)),
                                                             ss[0].loc);
    const_def_node->setTypeDescriptor(std::move(std::get<ir::TypeDescriptor>(ss[1].val)));
    if (const_def_node->getTypeDescriptor().isAuto()) {
        logger::debug(const_def_node->getLoc()).format("defining constant `{}`", const_def_node->getName());
    } else {
        logger::debug(const_def_node->getLoc())
            .format("defining constant `{}` of type `{}`", const_def_node->getName(),
                    const_def_node->getTypeDescriptor().getTypeString());
    }
    const_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[3].val)));
    pass->getCurrentScope().pushChildBack(std::move(const_def_node));
}

void defineVariable(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto var_def_node = std::make_unique<ir::VarDefNode>(std::string(std::get<std::string_view>(ss[1].val)), ss[1].loc);
    auto& type_desc = std::get<ir::TypeDescriptor>(ss[2].val);
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[0].val));
    var_def_node->setTypeDescriptor(std::move(type_desc));
    if (var_def_node->getTypeDescriptor().isAuto()) {
        logger::debug(var_def_node->getLoc()).format("defining variable `{}`", var_def_node->getName());
    } else {
        logger::debug(var_def_node->getLoc())
            .format("defining variable `{}` of type `{}`", var_def_node->getName(),
                    var_def_node->getTypeDescriptor().getTypeString());
    }
    var_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    pass->getCurrentScope().pushChildBack(std::move(var_def_node));
}

void specifyDefinitionType(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = std::move(std::get<ir::TypeDescriptor>(ss[1].val));
}

void noTypeSpecifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<ir::TypeDescriptor>(), loc = ss[-1].loc;
}

void makeTypeSpecifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto name = std::get<std::string_view>(ss[1].val);
    if (auto it = g_built_in_types.find(name); it != g_built_in_types.end()) {
        ss[0].val.emplace<ir::TypeDescriptor>(it->second);
    } else {
        auto& scope_desc = std::get<ir::ScopeDescriptor>(ss[0].val);
        if (scope_desc.getClass() != ir::ScopeClass::kInvalid) {
            if (auto* type_def_node = scope_desc.lookupName<ir::TypeDefNode>(name)) {
                ss[0].val.emplace<ir::TypeDescriptor>(ir::DataTypeClass::kDefinedDataType, type_def_node);
                return;
            } else {
                logger::error(ss[1].loc).format("undeclared type `{}`", name);
            }
        }
        ss[0].val.emplace<ir::TypeDescriptor>();
    }
}

void noTypeModifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) { ss[0].val = ir::DataTypeModifiers::kNone; }

void mutableTypeModifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = ir::DataTypeModifiers::kMutable;
}

void referenceTypeModifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = ir::DataTypeModifiers::kReference;
}

void mutableReferenceTypeModifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = ir::DataTypeModifiers::kMutable | ir::DataTypeModifiers::kReference;
}

void beginFuncProto(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto func_def_node = std::make_unique<ir::FuncDefNode>(std::string(std::get<std::string_view>(ss[-2].val)),
                                                           pass->getCurrentScope(), ss[-2].loc);
    logger::debug(func_def_node->getLoc()).format("defining function `{}`", func_def_node->getName());
    pass->setCurrentScope(*ss[-2].val.emplace<std::unique_ptr<ir::Node>>(std::move(func_def_node)));
}

void endFuncProto(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& func_def_node = util::cast<ir::FuncDefNode&>(*std::get<std::unique_ptr<ir::Node>>(ss[0].val));
    func_def_node.setTypeDescriptor(std::move(std::get<ir::TypeDescriptor>(ss[5].val)));
}

void declareFunc(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto func_def_node = std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val));
    logger::debug(loc).format("function `{}` declaration",
                              util::cast<ir::FuncDefNode&>(*func_def_node).getProtoString());
    pass->popCurrentScope();
    pass->getCurrentScope().pushChildBack(std::move(func_def_node));
}

void defineFunc(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto func_def_node = std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val));
    logger::debug(ss[0].loc + ss[1].loc)
        .format("function `{}` definition", util::cast<ir::FuncDefNode&>(*func_def_node).getProtoString());
    func_def_node->pushChildBack(std::move(std::get<std::unique_ptr<ir::Node>>(ss[2].val)));
    pass->popCurrentScope();
    pass->getCurrentScope().pushChildBack(std::move(func_def_node));
}

void addFuncFormalArg(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto formal_arg_def = std::make_unique<ir::DefNode>(std::string(std::get<std::string_view>(ss[1].val)), ss[1].loc);
    auto& type_desc = std::get<ir::TypeDescriptor>(ss[3].val);
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[0].val));
    formal_arg_def->setTypeDescriptor(std::move(type_desc));
    pass->getCurrentScope().pushChildBack(std::move(formal_arg_def));
}

void specifyReturnType(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& type_desc = std::get<ir::TypeDescriptor>(ss[2].val);
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[1].val));
    ss[0].val = std::move(type_desc);
}

void modifyReturnType(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& type_desc = ss[0].val.emplace<ir::TypeDescriptor>();
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[1].val));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_const, defineConst);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_variable, defineVariable);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_definition_type_specifier, specifyDefinitionType);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_no_type_specifier, noTypeSpecifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_make_type_specifier, makeTypeSpecifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_no_type_modifier, noTypeModifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_mutable_type_modifier, mutableTypeModifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_reference_type_modifier, referenceTypeModifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_mutable_reference_type_modifier, mutableReferenceTypeModifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_begin_func_proto, beginFuncProto);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_end_func_proto, endFuncProto);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_declare_func, declareFunc);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_define_func, defineFunc);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_add_func_formal_arg, addFuncFormalArg);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_ret_type_specifier, specifyReturnType);
DAISY_ADD_REDUCE_ACTION_HANDLER(parser_detail::act_ret_type_modifier, modifyReturnType);
