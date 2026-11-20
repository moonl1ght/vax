#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

NLOHMANN_JSON_NAMESPACE_BEGIN

// glm vectors are (de)serialized as plain arrays, e.g. vec3 -> [x, y, z]
template <glm::length_t L, typename T, glm::qualifier Q> struct adl_serializer<glm::vec<L, T, Q>> {
    template <typename BasicJsonType> static void to_json(BasicJsonType& json, const glm::vec<L, T, Q>& value) {
        json = BasicJsonType::array();
        for (glm::length_t i = 0; i < L; ++i) {
            json.push_back(value[i]);
        }
    }

    template <typename BasicJsonType> static void from_json(const BasicJsonType& json, glm::vec<L, T, Q>& value) {
        if (!json.is_array() || json.size() != static_cast<std::size_t>(L)) {
            throw detail::type_error::create(
                302, "expected array of size " + std::to_string(L) + " for glm::vec", &json
            );
        }
        for (glm::length_t i = 0; i < L; ++i) {
            value[i] = json.at(static_cast<std::size_t>(i)).template get<T>();
        }
    }
};

NLOHMANN_JSON_NAMESPACE_END
