#include "index_reader.h"
#include "bool_ops.h"
#include "query_parser.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static void usage() {
    std::cout << "Usage:\n"
              << "  search --index_dir <DIR> --query <EXPR> [--limit N]\n";
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

static std::vector<std::uint32_t> make_universe(const IndexReader& ir, std::uint32_t max_doc_id_hint = 1000000) {
    std::vector<std::uint32_t> u;
    u.reserve(max_doc_id_hint);
    for (std::uint32_t i = 1; i <= max_doc_id_hint; ++i) u.push_back(i);
    return u;
}

int main(int argc, char** argv) {
    std::string index_dir;
    std::string query;
    std::uint64_t limit = 0;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--index_dir") {
            if (!parse_arg(i, argc, argv, index_dir)) {
                std::cerr << "Missing value for --index_dir\n";
                return 2;
            }
        } else if (a == "--query") {
            if (!parse_arg(i, argc, argv, query)) {
                std::cerr << "Missing value for --query\n";
                return 2;
            }
        } else if (a == "--limit") {
            if (!parse_u64(i, argc, argv, limit)) {
                std::cerr << "Invalid value for --limit\n";
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

    if (index_dir.empty() || query.empty()) {
        usage();
        return 2;
    }

    IndexReader ir;
    if (!ir.open(index_dir)) {
        std::cerr << "Cannot open index_dir: " << index_dir << "\n";
        return 1;
    }

    std::vector<Tok> toks;
    std::vector<Tok> rpn;
    try {
        toks = tokenize_query(query);
        rpn = to_rpn(toks);
    } catch (const std::exception& e) {
        std::cerr << "Query parse error: " << e.what() << "\n";
        return 2;
    }

    std::vector<std::uint32_t> universe = make_universe(ir, 30000);

    std::vector<std::vector<std::uint32_t>> st;

    for (const Tok& t : rpn) {
        if (t.type == TokType::TERM) {
            std::uint32_t tid = 0;
            std::vector<std::uint32_t> docs;
            if (ir.term_to_id(t.text, tid)) {
                if (!ir.read_postings(tid, docs)) docs.clear();
            } else {
                docs.clear();
            }
            std::sort(docs.begin(), docs.end());
            st.push_back(std::move(docs));
        } else if (t.type == TokType::NOT) {
            if (st.empty()) {
                std::cerr << "Query error: NOT without operand\n";
                return 2;
            }
            auto a = std::move(st.back());
            st.pop_back();
            auto r = op_not(a, universe);
            st.push_back(std::move(r));
        } else if (t.type == TokType::AND || t.type == TokType::OR) {
            if (st.size() < 2) {
                std::cerr << "Query error: binary op without 2 operands\n";
                return 2;
            }
            auto b = std::move(st.back()); st.pop_back();
            auto a = std::move(st.back()); st.pop_back();
            std::vector<std::uint32_t> r = (t.type == TokType::AND) ? op_and(a, b) : op_or(a, b);
            st.push_back(std::move(r));
        }
    }

    if (st.size() != 1) {
        std::cerr << "Query error: invalid expression\n";
        return 2;
    }

    const auto& res = st.back();
    std::uint64_t n = (limit == 0) ? static_cast<std::uint64_t>(res.size())
                                   : std::min<std::uint64_t>(limit, static_cast<std::uint64_t>(res.size()));
    for (std::uint64_t i = 0; i < n; ++i) {
        std::cout << res[i] << "\n";
    }

    return 0;
}