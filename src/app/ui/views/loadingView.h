#pragma once

#include "view.h"
#include <optional>
#include <string>

namespace vax::app {
class LoadingView final : public View {
  public:
    LoadingView(vax::gfx::Renderer& renderer)
        : View(renderer) {};

    ~LoadingView() = default;

    void update(const vax::gfx::FrameTime& frameTime) override {};

    void updateUI(const vax::gfx::FrameTime& frameTime) override;

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
} // namespace vax::app
