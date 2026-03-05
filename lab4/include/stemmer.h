#pragma once
#include <string>

class Stemmer {
public:
    std::string stem(const std::string& token) const;

private:
    static bool ends_with(const std::string& s, const std::string& suf);
    static void strip_suffix(std::string& s, const std::string& suf);
};