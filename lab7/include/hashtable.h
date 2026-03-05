#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

#include "dynarray_u32.h"

struct HTEntry {
    std::string key;
    DynArrayU32 postings;
    bool used;
    bool deleted;
};

class HashTable {
public:
    HashTable();
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void reserve(std::size_t n);
    DynArrayU32* get_or_insert(const std::string& key);
    std::size_t size() const;

    const HTEntry* entries() const;
    std::size_t capacity() const;

private:
    HTEntry* table_;
    std::size_t cap_;
    std::size_t sz_;
    std::size_t filled_;

    void rehash(std::size_t new_cap);
    std::size_t probe_find_slot(const std::string& key) const;
    std::size_t probe_insert_slot(const std::string& key) const;

    static std::uint64_t hash64(const std::string& s);
};