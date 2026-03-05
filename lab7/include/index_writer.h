#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "hashtable.h"

struct DictItem {
    std::string term;
    std::uint32_t term_id;
};

class IndexWriter {
public:
    IndexWriter() = default;

    void write(const HashTable& ht, const std::string& out_dir);

private:
    static void ensure_dir(const std::string& dir);
    static void sort_unique_postings(DynArrayU32& a);
};