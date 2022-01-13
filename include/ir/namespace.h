#pragma once

#include "ir/named_object.h"
#include "uxs/algorithm.h"

#include <unordered_map>

namespace daisy {
namespace ir {

class Namespace : public util::rtti_mixin<Namespace, NamedObject> {
 private:
    using ObjectListType = uxs::intrusive_list<NamedObject, NamedObject::ListHookType, &NamedObject::list_hook_>;
    ObjectListType objects_;
    std::unordered_multimap<std::string_view, NamedObject*> name_table_;

 public:
    explicit Namespace(std::string name) : rtti_mixin_t(std::move(name)) {}

    bool empty() const NOEXCEPT { return objects_.empty(); }
    typename ObjectListType::size_type size() const NOEXCEPT { return objects_.size(); }
    typename ObjectListType::iterator begin() { return objects_.begin(); }
    typename ObjectListType::const_iterator begin() const { return objects_.begin(); }
    typename ObjectListType::iterator end() NOEXCEPT { return objects_.end(); }
    typename ObjectListType::const_iterator end() const NOEXCEPT { return objects_.end(); }
    typename ObjectListType::reference front() { return objects_.front(); }
    typename ObjectListType::const_reference front() const { return objects_.front(); }
    typename ObjectListType::reference back() { return objects_.back(); }
    typename ObjectListType::const_reference back() const { return objects_.back(); }

    template<typename Ty>
    Ty* findObject(std::string_view name) const {
        for (const auto& item : uxs::make_range(name_table_.equal_range(name))) {
            if (auto* typed_obj = util::cast<Ty>(item.second); typed_obj) { return typed_obj; }
        }
        return nullptr;
    }

    template<typename Ty>
    Ty& addObject(std::unique_ptr<Ty> obj) {
        auto* ptr = obj.get();
        assert(ptr);
        objects_.push_back(std::move(obj));
        ptr->namespace_ = this;
        name_table_.emplace(ptr->name_, ptr);
        return *ptr;
    }

    template<typename Ty, typename... Args>
    Ty& findOrAddObject(std::string name, Args&&... args) {
        if (auto* obj = findObject<Ty>(name); obj) { return *obj; }
        return addObject<Ty>(std::make_unique<Ty>(std::move(name), std::forward<Args>(args)...));
    }
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::Namespace);
