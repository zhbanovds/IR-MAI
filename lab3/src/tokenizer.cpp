#include "tokenizer.h"

bool Tokenizer::is_token_char(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    return false;
}

unsigned char Tokenizer::to_lower_ascii(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<unsigned char>(c - 'A' + 'a');
    return c;
}

std::vector<std::string> Tokenizer::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::string cur;
    cur.reserve(32);

    for (unsigned char uc : text) {
        if (is_token_char(uc)) {
            cur.push_back(static_cast<char>(to_lower_ascii(uc)));
        } else {
            if (cur.size() >= 2) {
                tokens.push_back(cur);
            }
            cur.clear();
        }
    }

    if (cur.size() >= 2) {
        tokens.push_back(cur);
    }

    return tokens;
}