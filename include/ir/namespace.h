#pragma once

#include "util/rtti.h"
#include "uxs/algorithm.h"

#include <unordered_map>

namespace daisy {
namespace ir {

class NamedNode;

class Namespace {
 private:
    using NameTableType = std::unordered_multimap<std::string_view, NamedNode*>;

 public:
    using value_type = typename NameTableType::value_type;
    using size_type = typename NameTableType::size_type;
    using difference_type = typename NameTableType::difference_type;
    using reference = typename NameTableType::reference;
    using const_reference = typename NameTableType::const_reference;
    using pointer = typename NameTableType::pointer;
    using const_pointer = typename NameTableType::const_pointer;
    using iterator = typename NameTableType::iterator;
    using const_iterator = typename NameTableType::const_iterator;

    bool empty() const NOEXCEPT { return name_table_.empty(); }
    size_type size() const NOEXCEPT { return name_table_.size(); }
    iterator begin() { return name_table_.begin(); }
    const_iterator begin() const { return name_table_.begin(); }
    iterator end() NOEXCEPT { return name_table_.end(); }
    const_iterator end() const NOEXCEPT { return name_table_.end(); }

    template<typename Ty>
    const Ty* findNode(std::string_view name) const {
        for (const auto& item : uxs::make_range(name_table_.equal_range(name))) {
            if (const auto* obj = util::cast<const Ty*>(item.second); obj) { return obj; }
        }
        return nullptr;
    }

    template<typename Ty>
    Ty* findNode(std::string_view name) {
        return const_cast<Ty*>(std::as_const(*this).findNode<Ty>(name));
    }

    template<typename Pred>
    const NamedNode* findNode(std::string_view name, Pred p) const {
        for (const auto& item : uxs::make_range(name_table_.equal_range(name))) {
            if (p(*item.second)) { return item.second; }
        }
        return nullptr;
    }

    template<typename Pred>
    NamedNode* findNode(std::string_view name, Pred p) {
        return const_cast<NamedNode*>(std::as_const(*this).findNode(name, p));
    }

    template<typename Ty, typename = std::enable_if_t<std::is_base_of_v<NamedNode, Ty>>>
    void addNode(Ty& obj) {
        name_table_.emplace(obj.getName(), &obj);
    }

 private:
    NameTableType name_table_;
};

}  // namespace ir
}  // namespace daisy
