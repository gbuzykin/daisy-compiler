#pragma once

#include "util/rtti.h"
#include "uxs/intrusive_list.h"

#include <memory>

namespace daisy {
namespace ir {

class Node : public util::rtti_mixin<Node> {
 private:
    Node* parent_ = nullptr;

    using ListHookType = uxs::intrusive_list_hook_t<Node, std::unique_ptr<Node>>;
    ListHookType list_hook_;

    using NodeListType = uxs::intrusive_list<Node, ListHookType, &Node::list_hook_>;
    NodeListType children_;

 public:
    virtual ~Node() = default;

    bool empty() const NOEXCEPT { return children_.empty(); }
    typename NodeListType::size_type size() const NOEXCEPT { return children_.size(); }
    typename NodeListType::iterator begin() { return children_.begin(); }
    typename NodeListType::const_iterator begin() const { return children_.begin(); }
    typename NodeListType::iterator end() NOEXCEPT { return children_.end(); }
    typename NodeListType::const_iterator end() const NOEXCEPT { return children_.end(); }
    typename NodeListType::reference front() { return children_.front(); }
    typename NodeListType::const_reference front() const { return children_.front(); }
    typename NodeListType::reference back() { return children_.back(); }
    typename NodeListType::const_reference back() const { return children_.back(); }

    template<typename Ty, typename = std::enable_if_t<std::is_base_of_v<Node, Ty>>>
    Ty& insertChild(NodeListType::const_iterator pos, std::unique_ptr<Ty> node) {
        auto* ptr = node.get();
        assert(ptr);
        children_.insert(pos, std::move(node));
        ptr->parent_ = this;
        return *ptr;
    }
};

}  // namespace ir
}  // namespace daisy

RTTI_DECLARE_TYPE_INFO(daisy::ir::Node);
