#include "dynarray_u32.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

DynArrayU32::DynArrayU32() : data(nullptr), size(0), capacity(0) {}

DynArrayU32::~DynArrayU32() {
    std::free(data);
    data = nullptr;
    size = 0;
    capacity = 0;
}

DynArrayU32::DynArrayU32(DynArrayU32&& other) noexcept
    : data(other.data), size(other.size), capacity(other.capacity) {
    other.data = nullptr;
    other.size = 0;
    other.capacity = 0;
}

DynArrayU32& DynArrayU32::operator=(DynArrayU32&& other) noexcept {
    if (this == &other) return *this;
    std::free(data);
    data = other.data;
    size = other.size;
    capacity = other.capacity;
    other.data = nullptr;
    other.size = 0;
    other.capacity = 0;
    return *this;
}

void DynArrayU32::reserve(std::size_t new_cap) {
    if (new_cap <= capacity) return;
    std::size_t bytes = new_cap * sizeof(std::uint32_t);
    void* p = std::realloc(data, bytes);
    if (!p) throw std::bad_alloc();
    data = static_cast<std::uint32_t*>(p);
    capacity = new_cap;
}

void DynArrayU32::push_back(std::uint32_t v) {
    if (size == capacity) {
        std::size_t new_cap = (capacity == 0) ? 8 : (capacity * 2);
        reserve(new_cap);
    }
    data[size++] = v;
}

std::uint32_t& DynArrayU32::operator[](std::size_t i) {
    return data[i];
}

const std::uint32_t& DynArrayU32::operator[](std::size_t i) const {
    return data[i];
}

void DynArrayU32::clear() {
    size = 0;
}