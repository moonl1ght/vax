#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>

namespace vax::scene {

struct CheckerboardTextureConfig final {
    int width = 512;
    int height = 512;
    int cellSize = 64;
};

using GeneratedTextureConfig = std::variant<CheckerboardTextureConfig>;

struct TextureConfig final {
    std::string name;
    std::string path;
    std::optional<GeneratedTextureConfig> generatedTextureConfig;

    bool isGenerated() const noexcept { return generatedTextureConfig.has_value(); }

    bool isValid() const noexcept { return isGenerated() || !path.empty(); }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CheckerboardTextureConfig, width, height, cellSize)

inline void to_json(nlohmann::json& json, const GeneratedTextureConfig& descriptor) {
    std::visit(
        [&json](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            json = value;
            if constexpr (std::is_same_v<T, CheckerboardTextureConfig>) {
                json["type"] = "checkerboard";
            }
        },
        descriptor
    );
}

namespace detail {
template <typename T> T valueOr(const nlohmann::json& json, const char* key, T fallback) noexcept {
    if (!json.is_object()) {
        return fallback;
    }
    const auto it = json.find(key);
    if (it == json.end()) {
        return fallback;
    }
    if constexpr (std::is_same_v<T, std::string>) {
        return it->is_string() ? it->template get<std::string>() : fallback;
    } else if constexpr (std::is_integral_v<T>) {
        return it->is_number_integer() ? it->template get<T>() : fallback;
    } else {
        static_assert(false, "unsupported type");
    }
}
} // namespace detail

inline std::optional<GeneratedTextureConfig> parseGeneratedTextureConfig(const nlohmann::json& json) noexcept {
    if (!json.is_object()) {
        return std::nullopt;
    }
    const std::string type = detail::valueOr(json, "type", std::string{});
    if (type == "checkerboard") {
        CheckerboardTextureConfig defaults;
        return CheckerboardTextureConfig{
            .width = detail::valueOr(json, "width", defaults.width),
            .height = detail::valueOr(json, "height", defaults.height),
            .cellSize = detail::valueOr(json, "cellSize", defaults.cellSize),
        };
    }
    return std::nullopt;
}

inline void to_json(nlohmann::json& json, const TextureConfig& descriptor) {
    json = {{"name", descriptor.name}};
    if (descriptor.generatedTextureConfig) {
        json["generatedTextureConfig"] = *descriptor.generatedTextureConfig;
    } else {
        json["path"] = descriptor.path;
    }
}

inline void from_json(const nlohmann::json& json, TextureConfig& descriptor) {
    descriptor = TextureConfig{};
    if (!json.is_object()) {
        return;
    }
    descriptor.name = detail::valueOr(json, "name", std::string{});
    descriptor.path = detail::valueOr(json, "path", std::string{});
    if (const auto it = json.find("generatedTextureConfig"); it != json.end()) {
        descriptor.generatedTextureConfig = parseGeneratedTextureConfig(*it);
    }
    if (descriptor.generatedTextureConfig) {
        descriptor.path.clear();
    }
}
} // namespace vax::scene
