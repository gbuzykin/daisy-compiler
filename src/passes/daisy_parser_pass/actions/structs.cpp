#include "../daisy_parser_pass.h"
#include "ir/nodes/struct_def_node.h"
#include "ir/nodes/var_def_node.h"
#include "logger.h"

using namespace daisy;

namespace {

void beginStructDef(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    const auto name = std::get<std::string_view>(ss[-2].val);
    auto& struct_def_node = pass->getCurrentScope().push_back(
        std::make_unique<ir::StructDefNode>(std::string(name), pass->getCurrentScope(), ss[-2].loc));
    auto& nmspace = pass->getCurrentScope().getNamespace();
    if (auto* nmspace_node = nmspace.findNode<ir::NamedScopeNode>(name)) {
        logger::error(ss[-2].loc).format("redefinition of `{}` as different kind of entity", name);
        logger::note(nmspace_node->getLoc()).format("previous definition is here");
    } else {
        nmspace.addNode(struct_def_node);
    }
    pass->setCurrentScope(struct_def_node);
}

void defineField(DaisyParserPass* pass, SymbolInfo* ss, SymbolLoc& loc) {
    auto field_def_node = std::make_unique<ir::VarDefNode>(std::string(std::get<std::string_view>(ss[1].val)),
                                                           ss[1].loc);
    auto& type_desc = std::get<ir::TypeDescriptor>(ss[3].val);
    type_desc.setModifiers(std::get<ir::DataTypeModifiers>(ss[0].val));
    field_def_node->setTypeDescriptor(std::move(type_desc));
    if (field_def_node->getTypeDescriptor().isAuto()) {
        logger::debug(field_def_node->getLoc()).format("defining field `{}`", field_def_node->getName());
    } else {
        logger::debug(field_def_node->getLoc())
            .format("defining field `{}` of type `{}`", field_def_node->getName(),
                    field_def_node->getTypeDescriptor().getTypeString());
    }
    pass->getCurrentScope().push_back(std::move(field_def_node));
}

}  // namespace

DAISY_ADD_REDUCE_ACTION_HANDLER(act_begin_struct_def, beginStructDef);
DAISY_ADD_REDUCE_ACTION_HANDLER(act_define_field, defineField);
