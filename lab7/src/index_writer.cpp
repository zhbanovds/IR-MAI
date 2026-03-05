#include "index_writer.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

static void write_u32(std::ofstream& out, std::uint32_t v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

static void write_u64(std::ofstream& out, std::uint64_t v) {
    out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

void IndexWriter::ensure_dir(const std::string& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
}

void IndexWriter::sort_unique_postings(DynArrayU32& a) {
    if (a.size == 0) return;
    std::sort(a.data, a.data + a.size);
    std::size_t w = 1;
    for (std::size_t i = 1; i < a.size; ++i) {
        if (a.data[i] != a.data[w - 1]) {
            a.data[w++] = a.data[i];
        }
    }
    a.size = w;
}

void IndexWriter::write(const HashTable& ht, const std::string& out_dir) {
    ensure_dir(out_dir);

    std::vector<DictItem> dict;
    dict.reserve(ht.size());

    const HTEntry* entries = ht.entries();
    std::size_t cap = ht.capacity();

    for (std::size_t i = 0; i < cap; ++i) {
        const HTEntry& e = entries[i];
        if (!e.used || e.deleted) continue;
        DictItem it;
        it.term = e.key;
        it.term_id = 0;
        dict.push_back(std::move(it));
    }

    std::sort(dict.begin(), dict.end(), [](const DictItem& a, const DictItem& b) {
        return a.term < b.term;
    });

    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(dict.size()); ++i) {
        dict[i].term_id = i;
    }

    std::ofstream dict_out(out_dir + "/dict.tsv", std::ios::binary);
    for (const auto& it : dict) {
        dict_out << it.term_id << '\t' << it.term << '\n';
    }

    std::ofstream postings_out(out_dir + "/postings.bin", std::ios::binary);
    std::ofstream offsets_out(out_dir + "/offsets.bin", std::ios::binary);

    std::vector<std::uint64_t> offsets(dict.size(), 0);

    for (const auto& it : dict) {
        std::size_t slot = ht.capacity();
        {
            std::uint64_t h = 1469598103934665603ull;
            for (unsigned char c : it.term) {
                h ^= static_cast<std::uint64_t>(c);
                h *= 1099511628211ull;
            }
            std::size_t mask = ht.capacity() - 1;
            std::size_t idx = static_cast<std::size_t>(h) & mask;
            const HTEntry* t = ht.entries();
            for (std::size_t step = 0; step < ht.capacity(); ++step) {
                const HTEntry& e = t[idx];
                if (!e.used) break;
                if (!e.deleted && e.key == it.term) {
                    slot = idx;
                    break;
                }
                idx = (idx + 1) & mask;
            }
        }

        const HTEntry& e = ht.entries()[slot];

        DynArrayU32 tmp;
        tmp.reserve(e.postings.size);
        for (std::size_t k = 0; k < e.postings.size; ++k) tmp.push_back(e.postings.data[k]);
        sort_unique_postings(tmp);

        std::uint64_t off = static_cast<std::uint64_t>(postings_out.tellp());
        offsets[it.term_id] = off;

        write_u32(postings_out, static_cast<std::uint32_t>(tmp.size));
        for (std::size_t k = 0; k < tmp.size; ++k) {
            write_u32(postings_out, tmp.data[k]);
        }
    }

    for (std::uint64_t off : offsets) {
        write_u64(offsets_out, off);
    }
}