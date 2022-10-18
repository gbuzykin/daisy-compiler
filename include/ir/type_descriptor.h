#pragma once

#include "uxs/utility.h"

namespace daisy {
namespace ir {

class TypeDefNode;

enum class DataTypeClass : unsigned {
    kBool = 0,
    kInt8,
    kUInt8,
    kInt16,
    kUInt16,
    kInt32,
    kUInt32,
    kInt64,
    kUInt64,
    kFloat32,
    kFloat64,
    kDefinedDataType
};

enum class DataTypeModifiers : unsigned {
    kNone = 0,
};
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(DataTypeModifiers, unsigned);

class TypeDescriptor {
 public:
    explicit TypeDescriptor(DataTypeClass cl, TypeDefNode* type_def = nullptr) : class_(cl), def_(type_def) {}

    DataTypeClass getClass() const { return class_; }
    DataTypeModifiers getModifiers() const { return modifiers_; }
    const TypeDefNode* getDefinitionNode() const { return def_; }
    TypeDefNode* getDefinitionNode() { return def_; }

 private:
    DataTypeClass class_;
    DataTypeModifiers modifiers_ = DataTypeModifiers::kNone;
    TypeDefNode* def_;
};

}  // namespace ir
}  // namespace daisy
