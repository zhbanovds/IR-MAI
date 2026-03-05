#pragma once
#include <string>
#include <vector>

enum class TokType {
    TERM,
    AND,
    OR,
    NOT,
    LPAREN,
    RPAREN
};

struct Tok {
    TokType type;
    std::string text;
};

std::vector<Tok> tokenize_query(const std::string& q);
std::vector<Tok> to_rpn(const std::vector<Tok>& toks);