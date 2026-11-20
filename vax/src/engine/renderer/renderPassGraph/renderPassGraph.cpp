#include "renderPassGraph.h"

using namespace vax::engine;

void RenderPassGraph::run(RenderPassNode::RunPassInfo& runPassInfo) {
    _rootNode->execute(runPassInfo);
}

std::shared_ptr<RenderPassNode> RenderPassGraph::findNode(const std::string& name) const {
    auto currentNode = _rootNode;
    while (currentNode) {
        if (currentNode->id() == name) {
            return currentNode;
        }
        currentNode = currentNode->next;
    }
    return nullptr;
}

void RenderPassGraph::setNodeEnabled(const std::string& name, bool enabled) {
    auto node = findNode(name);
    if (node) {
        node->setEnabled(enabled);
    }
}