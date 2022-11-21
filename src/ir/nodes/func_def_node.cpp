#include "ir/nodes/func_def_node.h"

#include "util/filter_iterator.h"

using namespace daisy;

RTTI_IMPLEMENT_TYPE_INFO(ir::FuncDefNode);

std::string ir::FuncDefNode::getProtoString() const {
    std::string s("func ");
    s += getName();
    s += '(';
    bool first_arg = true;
    for (const auto& child : util::make_filter_range<const DefNode>(*this)) {
        if (!first_arg) { s += ", "; }
        s += child.getTypeDescriptor().getTypeString();
        first_arg = false;
    }
    s += ')';
    if (!getTypeDescriptor().isAuto()) {
        s += " -> ";
        s += getTypeDescriptor().getTypeString();
    }
    return s;
}
