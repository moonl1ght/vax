#pragma once

#include "view.h"
#include <optional>
#include <string>

namespace vax::ui {
class LoadingView final : public View {
  public:
    LoadingView(vax::engine::Renderer& renderer)
        : View(renderer) {};

    ~LoadingView() = default;

    void update(const vax::engine::FrameTime& frameTime) override {};

    void updateUI(const vax::engine::FrameTime& frameTime) override;

    bool isShown() const noexcept { return _isShown; }

    void setShown(bool isShown) noexcept { _isShown = isShown; }

    void setMessage(std::string message) { _message = std::move(message); }

    void setProgress(std::optional<float> progress) noexcept { _progress = progress; }

  private:
    bool _isShown = false;
    std::string _message = "Loading";
    std::optional<float> _progress = std::nullopt;

    void _showIndicator() const;

    void _showProgress() const;
};
} // namespace vax::ui
