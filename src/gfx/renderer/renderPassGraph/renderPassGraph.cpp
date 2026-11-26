#include "renderPassGraph.h"

using namespace vax::gfx;

void RenderPassGraph::run(RenderPassNode::RunPassInfo& runPassInfo) { _rootNode->execute(runPassInfo); }

RenderPassNode* RenderPassGraph::findNode(const std::string& name) const {
    auto currentNode = _rootNode.get();
    while (currentNode) {
        if (currentNode->id() == name) {
            return currentNode->next.get();
        }
        currentNode = currentNode->next.get();
    }
    return nullptr;
}

void RenderPassGraph::setNodeEnabled(const std::string& name, bool enabled) {
    auto node = findNode(name);
    if (node) {
        node->setEnabled(enabled);
    }
}