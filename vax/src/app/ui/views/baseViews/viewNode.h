#pragma once

#include <view.h>

namespace vax::ui {
class ViewNode {
  public:
    ViewNode(std::unique_ptr<View> view)
        : view(std::move(view)) {}
    ~ViewNode() = default;

    std::unique_ptr<View> view;

    std::unique_ptr<ViewNode> child = nullptr;

    ViewNode* parent = nullptr;
};
} // namespace vax::ui