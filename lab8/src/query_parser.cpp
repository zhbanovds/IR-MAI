#include "query_parser.h"

#include <cctype>
#include <stdexcept>

static bool is_term_char(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '_') return true;
    return false;
}

static unsigned char to_lower_ascii(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<unsigned char>(c - 'A' + 'a');
    return c;
}

static int prec(TokType t) {
    if (t == TokType::NOT) return 3;
    if (t == TokType::AND) return 2;
    if (t == TokType::OR) return 1;
    return 0;
}

static bool is_op(TokType t) {
    return t == TokType::AND || t == TokType::OR || t == TokType::NOT;
}

std::vector<Tok> tokenize_query(const std::string& q) {
    std::vector<Tok> out;
    std::string cur;
    cur.reserve(32);

    auto flush_term = [&]() {
        if (cur.empty()) return;
        std::string up = cur;
        if (up == "and") out.push_back({TokType::AND, ""});
        else if (up == "or") out.push_back({TokType::OR, ""});
        else if (up == "not") out.push_back({TokType::NOT, ""});
        else out.push_back({TokType::TERM, cur});
        cur.clear();
    };

    for (unsigned char uc : q) {
        if (uc == '(') {
            flush_term();
            out.push_back({TokType::LPAREN, ""});
        } else if (uc == ')') {
            flush_term();
            out.push_back({TokType::RPAREN, ""});
        } else if (is_term_char(uc)) {
            cur.push_back(static_cast<char>(to_lower_ascii(uc)));
        } else {
            flush_term();
        }
    }
    flush_term();

    return out;
}

std::vector<Tok> to_rpn(const std::vector<Tok>& toks) {
    std::vector<Tok> out;
    std::vector<Tok> st;

    for (const Tok& t : toks) {
        if (t.type == TokType::TERM) {
            out.push_back(t);
        } else if (t.type == TokType::LPAREN) {
            st.push_back(t);
        } else if (t.type == TokType::RPAREN) {
            while (!st.empty() && st.back().type != TokType::LPAREN) {
                out.push_back(st.back());
                st.pop_back();
            }
            if (st.empty() || st.back().type != TokType::LPAREN) throw std::runtime_error("mismatched parens");
            st.pop_back();
        } else if (is_op(t.type)) {
            while (!st.empty() && is_op(st.back().type)) {
                TokType top = st.back().type;
                if (prec(top) > prec(t.type) || (prec(top) == prec(t.type) && t.type != TokType::NOT)) {
                    out.push_back(st.back());
                    st.pop_back();
                } else {
                    break;
                }
            }
            st.push_back(t);
        }
    }

    while (!st.empty()) {
        if (st.back().type == TokType::LPAREN || st.back().type == TokType::RPAREN) throw std::runtime_error("mismatched parens");
        out.push_back(st.back());
        st.pop_back();
    }

    return out;
}