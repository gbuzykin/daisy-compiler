#pragma once

#include "util/rtti.h"

#include <uxs/iterator.h>

#include <string_view>
#include <unordered_map>

namespace daisy {
namespace ir {

class Node;
class NamedNode;
class NamedScopeNode;
class DefNode;

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

    explicit Namespace(Node& parent_scope) : parent_scope_(&parent_scope) {}
    explicit Namespace(std::nullptr_t) : parent_scope_(nullptr) {}  // for root

    bool empty() const noexcept { return name_table_.empty(); }
    size_type size() const noexcept { return name_table_.size(); }
    iterator begin() noexcept { return name_table_.begin(); }
    const_iterator begin() const noexcept { return name_table_.begin(); }
    iterator end() noexcept { return name_table_.end(); }
    const_iterator end() const noexcept { return name_table_.end(); }

    const Node* getParentScope() const { return parent_scope_; }
    Node* getParentScope() { return parent_scope_; }

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

    template<typename Ty, typename Pred>
    const Ty* findNode(std::string_view name, Pred p) const {
        for (const auto& item : uxs::make_range(name_table_.equal_range(name))) {
            if (const auto* obj = util::cast<const Ty*>(item.second); obj && p(*obj)) { return obj; }
        }
        return nullptr;
    }

    template<typename Ty, typename Pred>
    Ty* findNode(std::string_view name, Pred p) {
        return const_cast<Ty*>(std::as_const(*this).findNode<Ty>(name, p));
    }

    template<typename Ty, typename = std::enable_if_t<std::is_base_of_v<NamedNode, Ty>>>
    Ty& addNode(Ty& obj) {
        name_table_.emplace(obj.getName(), &obj);
        return obj;
    }

    std::pair<NamedScopeNode*, bool> defineName(NamedScopeNode& named_scope);
    std::pair<DefNode*, bool> defineName(DefNode& def_node);

 private:
    Node* parent_scope_;
    NameTableType name_table_;
};

}  // namespace ir
}  // namespace daisy
