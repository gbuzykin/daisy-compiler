#include "ir/type_descriptor.h"

#include "ir/nodes/type_def_node.h"

using namespace daisy;

std::string ir::TypeDescriptor::getTypeString() const {
    std::string s;
    if (!!(modifiers_ & DataTypeModifiers::kReference)) { s += '&'; }
    if (!!(modifiers_ & DataTypeModifiers::kMutable)) { s += "mut"; }
    if (class_ == DataTypeClass::kAuto) { return s; }
    if (!s.empty()) { s += ' '; }
    switch (class_) {
        case DataTypeClass::kBool: s += "bool"; break;
        case DataTypeClass::kInt8: s += "i8"; break;
        case DataTypeClass::kUInt8: s += "u8"; break;
        case DataTypeClass::kInt16: s += "i16"; break;
        case DataTypeClass::kUInt16: s += "u16"; break;
        case DataTypeClass::kInt32: s += "i32"; break;
        case DataTypeClass::kUInt32: s += "u32"; break;
        case DataTypeClass::kInt64: s += "i64"; break;
        case DataTypeClass::kUInt64: s += "u64"; break;
        case DataTypeClass::kFloat32: s += "f32"; break;
        case DataTypeClass::kFloat64: s += "f64"; break;
        case DataTypeClass::kDefinedDataType: {
            assert(def_);
            s += def_->getGlobalName();
        } break;
        default: UNREACHABLE_CODE;
    }
    return s;
}
