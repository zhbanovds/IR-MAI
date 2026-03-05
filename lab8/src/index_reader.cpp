#include "index_reader.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

static bool read_u64_file(const std::string& path, std::vector<std::uint64_t>& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ios::end);
    std::streamoff n = in.tellg();
    if (n < 0) return false;
    if (n % static_cast<std::streamoff>(sizeof(std::uint64_t)) != 0) return false;
    std::size_t cnt = static_cast<std::size_t>(n / sizeof(std::uint64_t));
    out.resize(cnt);
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(cnt * sizeof(std::uint64_t)));
    return true;
}

bool IndexReader::load_dict(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    id_to_term_.clear();
    std::string line;

    std::uint32_t max_id = 0;
    bool any = false;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;
        std::string id_str = line.substr(0, tab);
        std::string term = line.substr(tab + 1);
        if (term.empty()) continue;

        std::uint32_t id = 0;
        try {
            id = static_cast<std::uint32_t>(std::stoul(id_str));
        } catch (...) {
            continue;
        }

        if (id >= id_to_term_.size()) id_to_term_.resize(id + 1);
        id_to_term_[id] = term;
        if (!any || id > max_id) max_id = id;
        any = true;
    }

    if (!any) return false;
    id_to_term_.resize(max_id + 1);
    return true;
}

bool IndexReader::load_offsets(const std::string& path) {
    offsets_.clear();
    return read_u64_file(path, offsets_);
}

bool IndexReader::open(const std::string& index_dir) {
    namespace fs = std::filesystem;
    fs::path dir(index_dir);
    fs::path dict = dir / "dict.tsv";
    fs::path offsets = dir / "offsets.bin";
    fs::path postings = dir / "postings.bin";

    if (!load_dict(dict.string())) return false;
    if (!load_offsets(offsets.string())) return false;

    postings_path_ = postings.string();
    return true;
}

std::uint32_t IndexReader::terms_count() const {
    return static_cast<std::uint32_t>(id_to_term_.size());
}

bool IndexReader::term_to_id(const std::string& term, std::uint32_t& term_id) const {
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(id_to_term_.size()); ++i) {
        if (id_to_term_[i] == term) {
            term_id = i;
            return true;
        }
    }
    return false;
}

static bool read_u32(std::ifstream& in, std::uint32_t& v) {
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return static_cast<bool>(in);
}

bool IndexReader::read_postings(std::uint32_t term_id, std::vector<std::uint32_t>& out_docs) const {
    if (term_id >= offsets_.size()) return false;

    std::ifstream in(postings_path_, std::ios::binary);
    if (!in) return false;

    std::uint64_t off = offsets_[term_id];
    in.seekg(static_cast<std::streamoff>(off), std::ios::beg);
    if (!in) return false;

    std::uint32_t df = 0;
    if (!read_u32(in, df)) return false;

    out_docs.clear();
    out_docs.resize(df);
    if (df > 0) {
        in.read(reinterpret_cast<char*>(out_docs.data()), static_cast<std::streamsize>(df * sizeof(std::uint32_t)));
        if (!in) return false;
    }
    return true;
}