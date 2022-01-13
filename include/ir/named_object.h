#pragma once

#include "util/rtti.h"
#include "uxs/intrusive_list.h"

#include <memory>

namespace daisy {
namespace ir {

class Namespace;

class NamedObject : public util::rtti_mixin<NamedObject> {
 public:
    explicit NamedObject(std::string name) : name_(std::move(name)) {}
    virtual ~NamedObject() = default;

    const Namespace* getNamespace() const { return namespace_; }
    Namespace* getNamespace() { return namespace_; }
    const std::string& getName() const { return name_; }

 private:
    friend class Namespace;
    std::string name_;
    Namespace* namespace_ = nullptr;

    using ListHookType = uxs::intrusive_list_hook_t<NamedObject, std::unique_ptr<NamedObject>>;
    ListHookType list_hook_;
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::NamedObject);
