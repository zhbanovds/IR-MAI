#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Entry {
    std::string term;
    std::uint64_t count;
};

static void usage() {
    std::cout << "Usage:\n"
              << "  zipf --in <tokens.tsv> --out <freq.tsv> [--zipf <zipf.tsv>] [--top <N>]\n";
}

static bool parse_arg(int& i, int argc, char** argv, std::string& out) {
    if (i + 1 >= argc) return false;
    out = argv[++i];
    return true;
}

static bool parse_u64(int& i, int argc, char** argv, std::uint64_t& out) {
    if (i + 1 >= argc) return false;
    try {
        out = static_cast<std::uint64_t>(std::stoull(argv[++i]));
    } catch (...) {
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    std::string in_path;
    std::string out_path;
    std::string zipf_path;
    std::uint64_t top_n = 0;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--in") {
            if (!parse_arg(i, argc, argv, in_path)) {
                std::cerr << "Missing value for --in\n";
                return 2;
            }
        } else if (a == "--out") {
            if (!parse_arg(i, argc, argv, out_path)) {
                std::cerr << "Missing value for --out\n";
                return 2;
            }
        } else if (a == "--zipf") {
            if (!parse_arg(i, argc, argv, zipf_path)) {
                std::cerr << "Missing value for --zipf\n";
                return 2;
            }
        } else if (a == "--top") {
            if (!parse_u64(i, argc, argv, top_n)) {
                std::cerr << "Invalid value for --top\n";
                return 2;
            }
        } else if (a == "--help" || a == "-h") {
            usage();
            return 0;
        } else {
            std::cerr << "Unknown arg: " << a << "\n";
            usage();
            return 2;
        }
    }

    if (in_path.empty() || out_path.empty()) {
        usage();
        return 2;
    }

    std::ifstream in(in_path, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file: " << in_path << "\n";
        return 1;
    }

    std::filesystem::path outp(out_path);
    if (!outp.parent_path().empty()) {
        std::error_code ec;
        std::filesystem::create_directories(outp.parent_path(), ec);
    }

    std::string line;
    std::vector<std::string> terms;
    terms.reserve(1'000'000);

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;
        std::string term = line.substr(tab + 1);
        if (term.empty()) continue;
        terms.push_back(std::move(term));
    }

    std::sort(terms.begin(), terms.end());

    std::vector<Entry> freq;
    freq.reserve(terms.size() / 4 + 1);

    for (std::size_t i = 0; i < terms.size();) {
        std::size_t j = i + 1;
        while (j < terms.size() && terms[j] == terms[i]) ++j;
        Entry e;
        e.term = terms[i];
        e.count = static_cast<std::uint64_t>(j - i);
        freq.push_back(std::move(e));
        i = j;
    }

    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open output file: " << out_path << "\n";
        return 1;
    }

    for (const auto& e : freq) {
        out << e.term << '\t' << e.count << '\n';
    }

    std::sort(freq.begin(), freq.end(), [](const Entry& a, const Entry& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.term < b.term;
    });

    if (!zipf_path.empty()) {
        std::filesystem::path zp(zipf_path);
        if (!zp.parent_path().empty()) {
            std::error_code ec;
            std::filesystem::create_directories(zp.parent_path(), ec);
        }
        std::ofstream z(zipf_path, std::ios::binary);
        if (!z) {
            std::cerr << "Cannot open zipf file: " << zipf_path << "\n";
            return 1;
        }
        for (std::size_t r = 0; r < freq.size(); ++r) {
            z << (r + 1) << '\t' << freq[r].count << '\n';
        }
    }

    if (top_n > 0) {
        std::uint64_t m = std::min<std::uint64_t>(top_n, static_cast<std::uint64_t>(freq.size()));
        for (std::uint64_t i = 0; i < m; ++i) {
            std::cout << (i + 1) << '\t' << freq[i].term << '\t' << freq[i].count << '\n';
        }
    }

    return 0;
}