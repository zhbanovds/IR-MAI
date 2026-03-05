#pragma once
#include <string>
#include <vector>

class Tokenizer {
public:
    std::vector<std::string> tokenize(const std::string& text) const;

private:
    static bool is_token_char(unsigned char c);
    static unsigned char to_lower_ascii(unsigned char c);
};