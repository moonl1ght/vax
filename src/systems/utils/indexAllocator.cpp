#include "indexAllocator.h"

using namespace vax;

std::optional<uint32_t> IndexAllocator::allocate() noexcept {
    uint32_t index;
    if (!_freeIndices.empty()) {
        index = _freeIndices.back();
        _freeIndices.pop_back();
    } else if (_next < _capacity) {
        index = _next++;
    } else {
        return std::nullopt;
    }
    _usedIndices[index] = true;
    ++_size;
    return index;
}

std::optional<uint32_t> IndexAllocator::allocate(uint32_t count) noexcept {
    if (count == 0) {
        return std::nullopt;
    }
    if (count == 1) {
        return allocate();
    }
    if (count > _capacity - _next) {
        return std::nullopt;
    }
    uint32_t first = _next;
    _next += count;
    for (uint32_t i = first; i < first + count; ++i) {
        _usedIndices[i] = true;
    }
    _size += count;
    return first;
}

bool IndexAllocator::release(uint32_t index) noexcept {
    if (index >= _capacity || !_usedIndices[index]) {
        return false;
    }
    _usedIndices[index] = false;
    --_size;
    _freeIndices.push_back(index);
    return true;
}

bool IndexAllocator::release(uint32_t first, uint32_t count) noexcept {
    bool released = true;
    for (uint32_t i = first; i < first + count; ++i) {
        released &= release(i);
    }
    return released;
}