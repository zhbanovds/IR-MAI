#include "hashtable.h"
#include <cstdlib>
#include <new>
#include <utility>

static std::size_t next_pow2(std::size_t x) {
    std::size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

HashTable::HashTable() : table_(nullptr), cap_(0), sz_(0), filled_(0) {}

HashTable::~HashTable() {
    if (table_) {
        for (std::size_t i = 0; i < cap_; ++i) {
            table_[i].~HTEntry();
        }
        std::free(table_);
    }
    table_ = nullptr;
    cap_ = 0;
    sz_ = 0;
    filled_ = 0;
}

std::size_t HashTable::size() const {
    return sz_;
}

const HTEntry* HashTable::entries() const {
    return table_;
}

std::size_t HashTable::capacity() const {
    return cap_;
}

std::uint64_t HashTable::hash64(const std::string& s) {
    std::uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ull;
    }
    return h;
}

void HashTable::reserve(std::size_t n) {
    std::size_t need = (n * 10) / 7 + 8;
    if (cap_ >= need) return;
    rehash(next_pow2(need));
}

void HashTable::rehash(std::size_t new_cap) {
    new_cap = next_pow2(new_cap);
    HTEntry* new_table = static_cast<HTEntry*>(std::malloc(new_cap * sizeof(HTEntry)));
    if (!new_table) throw std::bad_alloc();

    for (std::size_t i = 0; i < new_cap; ++i) {
        new (new_table + i) HTEntry{std::string(), DynArrayU32(), false, false};
    }

    HTEntry* old = table_;
    std::size_t old_cap = cap_;

    table_ = new_table;
    cap_ = new_cap;
    sz_ = 0;
    filled_ = 0;

    if (old) {
        for (std::size_t i = 0; i < old_cap; ++i) {
            if (old[i].used && !old[i].deleted) {
                DynArrayU32* dst = get_or_insert(old[i].key);
                *dst = std::move(old[i].postings);
            }
            old[i].~HTEntry();
        }
        std::free(old);
    }
}

std::size_t HashTable::probe_find_slot(const std::string& key) const {
    std::uint64_t h = hash64(key);
    std::size_t mask = cap_ - 1;
    std::size_t i = static_cast<std::size_t>(h) & mask;

    for (std::size_t step = 0; step < cap_; ++step) {
        const HTEntry& e = table_[i];
        if (!e.used) return cap_;
        if (!e.deleted && e.key == key) return i;
        i = (i + 1) & mask;
    }
    return cap_;
}

std::size_t HashTable::probe_insert_slot(const std::string& key) const {
    std::uint64_t h = hash64(key);
    std::size_t mask = cap_ - 1;
    std::size_t i = static_cast<std::size_t>(h) & mask;
    std::size_t first_del = cap_;

    for (std::size_t step = 0; step < cap_; ++step) {
        const HTEntry& e = table_[i];
        if (!e.used) return (first_del != cap_) ? first_del : i;
        if (e.deleted && first_del == cap_) first_del = i;
        if (!e.deleted && e.key == key) return i;
        i = (i + 1) & mask;
    }
    return (first_del != cap_) ? first_del : cap_;
}

DynArrayU32* HashTable::get_or_insert(const std::string& key) {
    if (cap_ == 0) rehash(1024);
    if ((filled_ + 1) * 10 >= cap_ * 7) {
        rehash(cap_ * 2);
    }

    std::size_t idx = probe_find_slot(key);
    if (idx != cap_) {
        return &table_[idx].postings;
    }

    idx = probe_insert_slot(key);
    if (idx == cap_) {
        rehash(cap_ * 2);
        idx = probe_insert_slot(key);
    }

    HTEntry& e = table_[idx];
    if (!e.used) {
        e.used = true;
        e.deleted = false;
        e.key = key;
        e.postings.clear();
        ++sz_;
        ++filled_;
        return &e.postings;
    }

    if (e.deleted) {
        e.deleted = false;
        e.key = key;
        e.postings.clear();
        ++sz_;
        return &e.postings;
    }

    return &e.postings;
}