#include "stemmer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static void usage() {
    std::cout << "Usage:\n"
              << "  stem --in <tokens.tsv> --out <stems.tsv>\n";
}

static bool parse_arg(int& i, int argc, char** argv, std::string& out) {
    if (i + 1 >= argc) return false;
    out = argv[++i];
    return true;
}

int main(int argc, char** argv) {
    std::string in_path;
    std::string out_path;

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

    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open output file: " << out_path << "\n";
        return 1;
    }

    Stemmer st;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;

        std::string doc_id = line.substr(0, tab);
        std::string token = line.substr(tab + 1);
        if (token.empty()) continue;

        std::string stem = st.stem(token);
        out << doc_id << '\t' << stem << '\n';
    }

    return 0;
}