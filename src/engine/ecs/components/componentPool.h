#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace vax::ecs {

class IComponentPool {
  public:
    virtual ~IComponentPool() = default;

    virtual bool contains(uint32_t index) const = 0;

    virtual void remove(uint32_t index) = 0;

    virtual void clear() = 0;

    virtual size_t size() const = 0;

    virtual std::span<const uint32_t> entities() const = 0;
};

template <typename T> class ComponentPool final : public IComponentPool {
  public:
    ComponentPool() = default;
    ~ComponentPool() override = default;

    ComponentPool(const ComponentPool& other) = delete;
    ComponentPool& operator=(const ComponentPool& other) = delete;
    ComponentPool(ComponentPool&& other) noexcept = default;
    ComponentPool& operator=(ComponentPool&& other) noexcept = default;

    template <typename... Args> T& emplace(uint32_t index, Args&&... args) {
        if (contains(index)) {
            T& component = _components[_sparseIndices[index]];
            component = T(std::forward<Args>(args)...);
            return component;
        }
        if (index >= _sparseIndices.size()) {
            _sparseIndices.resize(index + 1, NullComponentIndex);
        }
        _sparseIndices[index] = static_cast<uint32_t>(_components.size());
        _denseIndices.push_back(index);
        return _components.emplace_back(std::forward<Args>(args)...);
    }

    void remove(uint32_t index) override {
        if (!contains(index)) {
            return;
        }
        uint32_t slot = _sparseIndices[index];
        uint32_t lastSlot = static_cast<uint32_t>(_components.size() - 1);
        if (slot != lastSlot) {
            _components[slot] = std::move(_components[lastSlot]);
            _denseIndices[slot] = _denseIndices[lastSlot];
            _sparseIndices[_denseIndices[slot]] = slot;
        }
        _components.pop_back();
        _denseIndices.pop_back();
        _sparseIndices[index] = NullComponentIndex;
    }

    void clear() override {
        _sparseIndices.clear();
        _denseIndices.clear();
        _components.clear();
    }

    bool contains(uint32_t index) const override {
        return index < _sparseIndices.size() && _sparseIndices[index] != NullComponentIndex;
    }

    size_t size() const override { return _components.size(); }

    T& get(uint32_t index) {
        assert(contains(index));
        return _components[_sparseIndices[index]];
    }

    const T& get(uint32_t index) const {
        assert(contains(index));
        return _components[_sparseIndices[index]];
    }

    T* tryGet(uint32_t index) { return contains(index) ? &_components[_sparseIndices[index]] : nullptr; }

    const T* tryGet(uint32_t index) const { return contains(index) ? &_components[_sparseIndices[index]] : nullptr; }

    std::span<const uint32_t> entities() const override { return _denseIndices; }

    std::span<T> components() { return _components; }

    std::span<const T> components() const { return _components; }

  private:
    static constexpr uint32_t NullComponentIndex = std::numeric_limits<uint32_t>::max();

    std::vector<uint32_t> _sparseIndices;
    std::vector<uint32_t> _denseIndices;
    std::vector<T> _components;
};
} // namespace vax::ecs
