#include "hashtable.h"
#include "index_writer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static void usage() {
    std::cout << "Usage:\n"
              << "  indexer --in <tokens_or_stems.tsv> --out_dir <DIR>\n";
}

static bool parse_arg(int& i, int argc, char** argv, std::string& out) {
    if (i + 1 >= argc) return false;
    out = argv[++i];
    return true;
}

int main(int argc, char** argv) {
    std::string in_path;
    std::string out_dir;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--in") {
            if (!parse_arg(i, argc, argv, in_path)) {
                std::cerr << "Missing value for --in\n";
                return 2;
            }
        } else if (a == "--out_dir") {
            if (!parse_arg(i, argc, argv, out_dir)) {
                std::cerr << "Missing value for --out_dir\n";
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

    if (in_path.empty() || out_dir.empty()) {
        usage();
        return 2;
    }

    std::ifstream in(in_path, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file: " << in_path << "\n";
        return 1;
    }

    HashTable ht;
    ht.reserve(200000);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;

        std::string doc_str = line.substr(0, tab);
        std::string term = line.substr(tab + 1);
        if (term.empty()) continue;

        std::uint32_t doc_id = 0;
        try {
            doc_id = static_cast<std::uint32_t>(std::stoul(doc_str));
        } catch (...) {
            continue;
        }

        DynArrayU32* p = ht.get_or_insert(term);
        p->push_back(doc_id);
    }

    IndexWriter w;
    w.write(ht, out_dir);

    return 0;
}