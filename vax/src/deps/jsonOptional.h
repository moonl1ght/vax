#pragma once

#include <nlohmann/json.hpp>
#include <optional>

NLOHMANN_JSON_NAMESPACE_BEGIN

template <typename T>
struct adl_serializer<std::optional<T>> {
    template <typename BasicJsonType>
    static void to_json(BasicJsonType& json, const std::optional<T>& value) {
        if (value) {
            json = *value;
        } else {
            json = nullptr;
        }
    }

    template <typename BasicJsonType>
    static void from_json(const BasicJsonType& json, std::optional<T>& value) {
        if (json.is_null()) {
            value = std::nullopt;
        } else {
            value = json.template get<T>();
        }
    }
};

NLOHMANN_JSON_NAMESPACE_END
