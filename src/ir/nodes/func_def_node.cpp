#include "ir/nodes/func_def_node.h"

#include "util/filter_iterator.h"
#include "uxs/zip_iterator.h"

using namespace daisy;

RTTI_IMPLEMENT_TYPE_INFO(ir::FuncDefNode);

std::string ir::FuncDefNode::getProtoString() const {
    std::string s("func ");
    s += getGlobalName();
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

ir::FuncProtoCompareResult ir::FuncDefNode::compareProto(const FuncDefNode& other_proto) const {
    bool arg_type_diff = false;
    auto it = uxs::for_loop(
        uxs::zip(util::make_filter_range<const DefNode>(*this), util::make_filter_range<const DefNode>(other_proto)),
        [&arg_type_diff](const auto& pair) {
            arg_type_diff = arg_type_diff ||
                            std::get<0>(pair).getTypeDescriptor() != std::get<1>(pair).getTypeDescriptor();
        });

    if (it.base<0>().base() != end() || it.base<1>().base() != other_proto.end()) {
        return FuncProtoCompareResult::kArgCountDiff;
    } else if (arg_type_diff) {
        return FuncProtoCompareResult::kArgTypeDiff;
    } else if (getTypeDescriptor() != other_proto.getTypeDescriptor()) {
        return FuncProtoCompareResult::kRetTypeDiff;
    }

    return FuncProtoCompareResult::kEqual;
}
