#include "tokenizer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

static void usage() {
    std::cout << "Usage:\n"
              << "  tokenize --docs_dir <DIR> --out <FILE>\n";
}

static bool parse_arg(int& i, int argc, char** argv, std::string& out) {
    if (i + 1 >= argc) return false;
    out = argv[++i];
    return true;
}

int main(int argc, char** argv) {
    std::string docs_dir;
    std::string out_path;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--docs_dir") {
            if (!parse_arg(i, argc, argv, docs_dir)) {
                std::cerr << "Missing value for --docs_dir\n";
                return 2;
            }
        } else if (a == "--out") {
            if (!parse_arg(i, argc, argv, out_path)) {
                std::cerr << "Missing value for --out\n";
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

    if (docs_dir.empty() || out_path.empty()) {
        usage();
        return 2;
    }

    namespace fs = std::filesystem;
    if (!fs::exists(docs_dir) || !fs::is_directory(docs_dir)) {
        std::cerr << "docs_dir is not a directory: " << docs_dir << "\n";
        return 1;
    }

    fs::path outp(out_path);
    if (!outp.parent_path().empty()) {
        std::error_code ec;
        fs::create_directories(outp.parent_path(), ec);
    }

    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open output file: " << out_path << "\n";
        return 1;
    }

    Tokenizer tok;

    std::vector<fs::path> files;
    for (const auto& it : fs::directory_iterator(docs_dir)) {
        if (!it.is_regular_file()) continue;
        auto p = it.path();
        if (p.extension() == ".txt") files.push_back(p);
    }

    std::sort(files.begin(), files.end());

    std::string content;
    for (const auto& p : files) {
        std::ifstream in(p, std::ios::binary);
        if (!in) continue;

        content.assign((std::istreambuf_iterator<char>(in)),
                       (std::istreambuf_iterator<char>()));

        auto tokens = tok.tokenize(content);

        std::string stem = p.stem().string();
        for (const auto& t : tokens) {
            out << stem << '\t' << t << '\n';
        }
    }

    return 0;
}