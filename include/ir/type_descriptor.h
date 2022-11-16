#pragma once

#include "uxs/utility.h"

#include <string>

namespace daisy {
namespace ir {

class TypeDefNode;

enum class DataTypeClass : unsigned {
    kAuto = 0,
    kBool,
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

enum class DataTypeModifiers : unsigned { kNone = 0, kMutable = 1, kReference = 2 };
UXS_IMPLEMENT_BITWISE_OPS_FOR_ENUM(DataTypeModifiers, unsigned);

class TypeDescriptor {
 public:
    explicit TypeDescriptor(DataTypeClass cl = DataTypeClass::kAuto) : class_(cl) {}
    TypeDescriptor(DataTypeClass cl, TypeDefNode* type_def) : class_(cl), def_(type_def) {}

    DataTypeClass getClass() const { return class_; }
    DataTypeModifiers getModifiers() const { return modifiers_; }
    void setModifiers(DataTypeModifiers modifiers) { modifiers_ = modifiers; }
    bool isAuto() const { return class_ == DataTypeClass::kAuto && modifiers_ == DataTypeModifiers::kNone; }
    const TypeDefNode* getDefinitionNode() const { return def_; }
    TypeDefNode* getDefinitionNode() { return def_; }
    std::string getTypeString() const;

 private:
    DataTypeClass class_;
    DataTypeModifiers modifiers_ = DataTypeModifiers::kNone;
    TypeDefNode* def_ = nullptr;
};

}  // namespace ir
}  // namespace daisy
