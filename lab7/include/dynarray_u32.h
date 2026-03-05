#pragma once
#include <cstdint>
#include <cstddef>

struct DynArrayU32 {
    std::uint32_t* data;
    std::size_t size;
    std::size_t capacity;

    DynArrayU32();
    ~DynArrayU32();

    DynArrayU32(const DynArrayU32&) = delete;
    DynArrayU32& operator=(const DynArrayU32&) = delete;

    DynArrayU32(DynArrayU32&& other) noexcept;
    DynArrayU32& operator=(DynArrayU32&& other) noexcept;

    void reserve(std::size_t new_cap);
    void push_back(std::uint32_t v);
    std::uint32_t& operator[](std::size_t i);
    const std::uint32_t& operator[](std::size_t i) const;

    void clear();
};