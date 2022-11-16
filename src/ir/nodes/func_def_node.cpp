#include "ir/nodes/func_def_node.h"

using namespace daisy;
using namespace ir;

RTTI_IMPLEMENT_TYPE_INFO(FuncDefNode);

std::string FuncDefNode::getProtoString() const {
    std::string s("func ");
    s += getName();
    s += '(';
    bool first_arg = true;
    for (const auto& child : *this) {
        if (const auto* def_node = util::cast<const DefNode*>(&child)) {
            if (!first_arg) { s += ", "; }
            s += def_node->getTypeDescriptor().getTypeString();
            first_arg = false;
        }
    }
    s += ')';
    if (!getTypeDescriptor().isAuto()) {
        s += " -> ";
        s += getTypeDescriptor().getTypeString();
    }
    return s;
}
