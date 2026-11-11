#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace vax {

class IndexAllocator final {
  public:
    explicit IndexAllocator(uint32_t capacity) noexcept
        : _capacity(capacity)
        , _usedIndices(capacity, false) {}

    ~IndexAllocator() noexcept = default;

    IndexAllocator(const IndexAllocator&) = delete;
    IndexAllocator& operator=(const IndexAllocator&) = delete;
    IndexAllocator(IndexAllocator&&) noexcept = delete;
    IndexAllocator& operator=(IndexAllocator&&) noexcept = delete;

    std::optional<uint32_t> allocate() noexcept;

    std::optional<uint32_t> allocate(uint32_t count) noexcept;

    bool release(uint32_t index) noexcept;

    bool release(uint32_t first, uint32_t count) noexcept;

    uint32_t capacity() const noexcept { return _capacity; }

    uint32_t size() const noexcept { return _size; }

  private:
    uint32_t _capacity = 0;
    uint32_t _next = 0;
    uint32_t _size = 0;
    std::vector<uint32_t> _freeIndices;
    std::vector<bool> _usedIndices;
};

} // namespace vax