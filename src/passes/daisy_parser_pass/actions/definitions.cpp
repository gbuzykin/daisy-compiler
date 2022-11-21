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
    const auto name = std::get<std::string_view>(ss[0].val);
    auto& const_def_node = pass->getCurrentScope().push_back(
        std::make_unique<ir::ConstDefNode>(std::string(name), ss[0].loc));
    const_def_node.setTypeDescriptor(std::move(std::get<ir::TypeDescriptor>(ss[1].val)));
    const_def_node.push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[3].val)));
    if (const_def_node.getTypeDescriptor().isAuto()) {
        logger::debug(const_def_node.getLoc()).format("defining constant `{}`", name);
    } else {
        logger::debug(const_def_node.getLoc())
            .format("defining constant `{}` of type `{}`", name, const_def_node.getTypeDescriptor().getTypeString());
    }
    pass->getCurrentScope().getNamespace().defineName(const_def_node);
}

void defineVariable(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[1].val);
    auto& var_def_node = pass->getCurrentScope().push_back(
        std::make_unique<ir::VarDefNode>(std::string(name), ss[1].loc));
    auto& type_desc = var_def_node.setTypeDescriptor(std::move(std::get<ir::TypeDescriptor>(ss[2].val)));
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[0].val));
    var_def_node.push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[4].val)));
    if (var_def_node.getTypeDescriptor().isAuto()) {
        logger::debug(var_def_node.getLoc()).format("defining variable `{}`", name);
    } else {
        logger::debug(var_def_node.getLoc())
            .format("defining variable `{}` of type `{}`", name, var_def_node.getTypeDescriptor().getTypeString());
    }
    pass->getCurrentScope().getNamespace().defineName(var_def_node);
}

void makeTypeSpecifier(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[1].val);
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

void declareFunc(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    pass->popCurrentScope();

    auto& func_def_node = util::cast<ir::FuncDefNode&>(
        pass->getCurrentScope().push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[1].val))));
    logger::debug(ss[0].loc + ss[1].loc).format("function `{}` declaration", func_def_node.getProtoString());

    ir::FuncProtoCompareResult func_proto_compare_result = ir::FuncProtoCompareResult::kEqual;
    auto* existing_def_node = pass->getCurrentScope().getNamespace().findNode<ir::FuncDefNode>(
        func_def_node.getName(), [&func_def_node, &func_proto_compare_result](auto& def_node) {
            return (func_proto_compare_result = def_node.compareProto(func_def_node)) <=
                   ir::FuncProtoCompareResult::kRetTypeDiff;
        });
    if (!existing_def_node) {  // declare or overload function
        pass->getCurrentScope().getNamespace().addNode(func_def_node);
    } else if (func_proto_compare_result != ir::FuncProtoCompareResult::kEqual) {
        logger::error(func_def_node.getLoc())
            .format("functions that differ only in their return type cannot be overloaded");
        logger::note(existing_def_node->getLoc()).format("previous declaration is here");
    } else if (func_def_node.isDefined()) {
        if (!existing_def_node->isDefined()) {  // move definition to existing function declaration
            existing_def_node->push_back(func_def_node.extract(func_def_node.back()));
            existing_def_node->setDefined(func_def_node.getDefinitionLoc());
        } else {
            logger::error(func_def_node.getLoc()).format("redefinition of `{}`", func_def_node.getName());
            logger::note(existing_def_node->getDefinitionLoc()).format("previous definition is here");
        }
    }
}

void defineFunc(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& func_def_node = util::cast<ir::FuncDefNode&>(*std::get<std::unique_ptr<ir::Node>>(ss[-2].val));
    func_def_node.push_back(std::move(std::get<std::unique_ptr<ir::Node>>(ss[-1].val)));
    func_def_node.setDefined(func_def_node.getLoc());
    logger::debug(ss[-3].loc + ss[-2].loc).format("defining function `{}`", func_def_node.getProtoString());
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(act_define_const, defineConst);
DAISY_ADD_REDUCE_ACTION_HANDLER(act_define_variable, defineVariable);
DAISY_ADD_REDUCE_ACTION_HANDLER(act_make_type_specifier, makeTypeSpecifier);
DAISY_ADD_REDUCE_ACTION_HANDLER(act_declare_func, declareFunc);
DAISY_ADD_REDUCE_ACTION_HANDLER(act_define_func, defineFunc);

DAISY_ADD_REDUCE_ACTION_HANDLER(act_set_ret_type, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& func_def_node = util::cast<ir::FuncDefNode&>(*std::get<std::unique_ptr<ir::Node>>(ss[0].val));
    func_def_node.setTypeDescriptor(std::move(std::get<ir::TypeDescriptor>(ss[5].val)));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_add_func_formal_arg, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& formal_arg_def = pass->getCurrentScope().push_back(
        std::make_unique<ir::DefNode>(std::string(std::get<std::string_view>(ss[1].val)), ss[1].loc));
    auto& type_desc = formal_arg_def.setTypeDescriptor(std::get<ir::TypeDescriptor>(ss[3].val));
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[0].val));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_begin_func_decl, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    pass->setCurrentScope(*ss[-2].val.emplace<std::unique_ptr<ir::Node>>(std::make_unique<ir::FuncDefNode>(
        std::string(std::get<std::string_view>(ss[-2].val)), pass->getCurrentScope(), ss[-2].loc)));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_definition_type_specifier,
                                [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
                                    ss[0].val = std::move(std::get<ir::TypeDescriptor>(ss[1].val));
                                });

DAISY_ADD_REDUCE_ACTION_HANDLER(act_no_type_specifier, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<ir::TypeDescriptor>();
    loc = ss[-1].loc;
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_no_type_modifier, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = ir::DataTypeModifiers::kNone;
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_mutable_type_modifier, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = ir::DataTypeModifiers::kMutable;
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_reference_type_modifier, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val = ir::DataTypeModifiers::kReference;
});
DAISY_ADD_REDUCE_ACTION_HANDLER(act_mutable_reference_type_modifier,
                                [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
                                    ss[0].val = ir::DataTypeModifiers::kMutable | ir::DataTypeModifiers::kReference;
                                });

DAISY_ADD_REDUCE_ACTION_HANDLER(act_ret_type_specifier, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto& type_desc = ss[0].val.emplace<ir::TypeDescriptor>(std::move(std::get<ir::TypeDescriptor>(ss[2].val)));
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[1].val));
});

DAISY_ADD_REDUCE_ACTION_HANDLER(act_ret_type_modifier, [](DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    ss[0].val.emplace<ir::TypeDescriptor>().setModifiers(std::get<ir::DataTypeModifiers>(ss[1].val));
});
