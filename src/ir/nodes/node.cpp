#include "ir/nodes/node.h"

using namespace daisy;

RTTI_IMPLEMENT_TYPE_INFO(ir::Node);

const ir::Namespace& ir::Node::findNamespace() const {
    const Node* node = this;
    for (; node; node = node->parent_) {
        if (node->namespace_) { break; }
    }
    assert(node);
    return *node->namespace_;
}
