#pragma once

#include "ir/nodes/node.h"

namespace daisy {
namespace ir {

enum class ScopeClass : unsigned { kLocal = 0, kSpecified, kInvalid };

class ScopeDescriptor {
 public:
    explicit ScopeDescriptor(ScopeClass cl) : class_(cl) {}
    ScopeDescriptor(ScopeClass cl, Node& scope) : class_(cl), scope_(&scope) {}

    ScopeClass getClass() const { return class_; }
    template<typename Ty>
    const Ty* lookupName(std::string_view name) const;
    template<typename Ty>
    Ty* lookupName(std::string_view name) {
        return const_cast<Ty*>(std::as_const(*this).lookupName<Ty>(name));
    }

 private:
    ScopeClass class_;
    Node* scope_ = nullptr;
};

template<typename Ty>
const Ty* ScopeDescriptor::lookupName(std::string_view name) const {
    assert(!name.empty());
    switch (class_) {
        case ScopeClass::kLocal: {
            assert(scope_);
            const auto* scope = scope_;
            do {
                auto& nmspace = scope->getNamespace();
                if (const auto* found_node = nmspace.findNode<Ty>(name)) { return found_node; }
                scope = nmspace.getParentScope();
            } while (scope);
        } break;
        case ScopeClass::kSpecified: {
            assert(scope_);
            return scope_->getNamespace().findNode<Ty>(name);
        } break;
        default: break;
    }
    return nullptr;
}

}  // namespace ir
}  // namespace daisy
