#pragma once

#include "common/symbol_loc.h"
#include "ir/namespace.h"
#include "uxs/intrusive_list.h"

#include <memory>

namespace daisy {
namespace ir {

class Node : public util::rtti_mixin<Node> {
 private:
    Node* parent_ = nullptr;

    using ChildListHookType = uxs::intrusive_list_hook_t<Node, std::unique_ptr<Node>>;
    ChildListHookType child_list_hook_;

    using ChildListType = uxs::intrusive_list<Node, ChildListHookType, &Node::child_list_hook_>;
    ChildListType children_;

    SymbolLoc loc_;

    std::unique_ptr<Namespace> namespace_;

 public:
    using value_type = typename ChildListType::value_type;
    using size_type = typename ChildListType::size_type;
    using difference_type = typename ChildListType::difference_type;
    using reference = typename ChildListType::reference;
    using const_reference = typename ChildListType::const_reference;
    using pointer = typename ChildListType::pointer;
    using const_pointer = typename ChildListType::const_pointer;
    using iterator = typename ChildListType::iterator;
    using const_iterator = typename ChildListType::const_iterator;

    Node() = default;
    explicit Node(const SymbolLoc& loc) : loc_(loc) {}
    explicit Node(std::unique_ptr<Namespace> nmspace, const SymbolLoc& loc = {})
        : loc_(loc), namespace_(std::move(nmspace)) {}

    bool empty() const noexcept { return children_.empty(); }
    size_type size() const noexcept { return children_.size(); }
    iterator begin() noexcept { return children_.begin(); }
    const_iterator begin() const noexcept { return children_.begin(); }
    iterator end() noexcept { return children_.end(); }
    const_iterator end() const noexcept { return children_.end(); }
    reference front() { return children_.front(); }
    const_reference front() const { return children_.front(); }
    reference back() { return children_.back(); }
    const_reference back() const { return children_.back(); }

    const_pointer getParent() const { return parent_; }
    pointer getParent() { return parent_; }
    const SymbolLoc& getLoc() const { return loc_; }

    const Namespace& getNamespace() const {
        assert(namespace_);
        return *namespace_;
    }
    Namespace& getNamespace() {
        assert(namespace_);
        return *namespace_;
    }

    template<typename Ty, typename = std::enable_if_t<std::is_base_of_v<Node, Ty>>>
    Ty& insertChild(const_iterator pos, std::unique_ptr<Ty> node) {
        auto* ptr = node.get();
        assert(ptr);
        children_.insert(pos, std::move(node));
        ptr->parent_ = this;
        return *ptr;
    }

    template<typename Ty, typename = std::enable_if_t<std::is_base_of_v<Node, Ty>>>
    Ty& pushChildBack(std::unique_ptr<Ty> node) {
        return insertChild(children_.end(), std::move(node));
    }

    std::unique_ptr<Node> extract(Node& node) { return children_.extract(children_.to_iterator(node)); }
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::Node);
