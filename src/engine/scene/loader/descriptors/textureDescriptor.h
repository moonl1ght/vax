#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>

namespace vax::engine {

struct CheckerboardTextureDescriptor final {
    int width = 512;
    int height = 512;
    int cellSize = 64;
};

using GeneratedTextureDescriptor = std::variant<CheckerboardTextureDescriptor>;

struct TextureDescriptor final {
    std::string name;
    std::string path;
    std::optional<GeneratedTextureDescriptor> generatedTextureDescriptor;

    bool isGenerated() const noexcept { return generatedTextureDescriptor.has_value(); }

    bool isValid() const noexcept { return isGenerated() || !path.empty(); }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CheckerboardTextureDescriptor, width, height, cellSize)

inline void to_json(nlohmann::json& json, const GeneratedTextureDescriptor& descriptor) {
    std::visit(
        [&json](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            json = value;
            if constexpr (std::is_same_v<T, CheckerboardTextureDescriptor>) {
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

inline std::optional<GeneratedTextureDescriptor> parseGeneratedTextureDescriptor(const nlohmann::json& json) noexcept {
    if (!json.is_object()) {
        return std::nullopt;
    }
    const std::string type = detail::valueOr(json, "type", std::string{});
    if (type == "checkerboard") {
        CheckerboardTextureDescriptor defaults;
        return CheckerboardTextureDescriptor{
            .width = detail::valueOr(json, "width", defaults.width),
            .height = detail::valueOr(json, "height", defaults.height),
            .cellSize = detail::valueOr(json, "cellSize", defaults.cellSize),
        };
    }
    return std::nullopt;
}

inline void to_json(nlohmann::json& json, const TextureDescriptor& descriptor) {
    json = {{"name", descriptor.name}};
    if (descriptor.generatedTextureDescriptor) {
        json["generatedTextureDescriptor"] = *descriptor.generatedTextureDescriptor;
    } else {
        json["path"] = descriptor.path;
    }
}

inline void from_json(const nlohmann::json& json, TextureDescriptor& descriptor) {
    descriptor = TextureDescriptor{};
    if (!json.is_object()) {
        return;
    }
    descriptor.name = detail::valueOr(json, "name", std::string{});
    descriptor.path = detail::valueOr(json, "path", std::string{});
    if (const auto it = json.find("generatedTextureDescriptor"); it != json.end()) {
        descriptor.generatedTextureDescriptor = parseGeneratedTextureDescriptor(*it);
    }
    if (descriptor.generatedTextureDescriptor) {
        descriptor.path.clear();
    }
}
} // namespace vax::engine
