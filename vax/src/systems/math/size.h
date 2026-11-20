#pragma once

#include <concepts>
#include <vulkan/vulkan.h>

namespace vax::math {
template <typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

template <typename T>
    requires arithmetic<T>
struct Size_t {
    T width;
    T height;

    static Size_t zero() { return Size_t(0, 0); }

    Size_t(T width, T height)
        : width(width)
        , height(height) {}

    Size_t(VkExtent2D extent)
        : width(static_cast<T>(extent.width))
        , height(static_cast<T>(extent.height)) {}

    float whRatio() const noexcept {
        if (height == 0) {
            return 0;
        }
        return static_cast<float>(width) / static_cast<float>(height);
    }
    float hwRatio() const noexcept {
        if (width == 0) {
            return 0;
        }
        return static_cast<float>(height) / static_cast<float>(width);
    }

    VkExtent2D toExtent2D() const noexcept {
        return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    VkExtent3D toExtent3D() const noexcept {
        return VkExtent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    }
};

typedef Size_t<uint32_t> SizeUI;
typedef Size_t<double> SizeD;
typedef Size_t<float> SizeF;
typedef SizeD Size;
} // namespace vax::math