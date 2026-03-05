#pragma once
#include <cstdint>
#include <string>
#include <vector>

class IndexReader {
public:
    bool open(const std::string& index_dir);

    bool term_to_id(const std::string& term, std::uint32_t& term_id) const;

    bool read_postings(std::uint32_t term_id, std::vector<std::uint32_t>& out_docs) const;

    std::uint32_t terms_count() const;

private:
    std::vector<std::string> id_to_term_;
    std::vector<std::uint64_t> offsets_;
    std::string postings_path_;

    bool load_dict(const std::string& path);
    bool load_offsets(const std::string& path);
};