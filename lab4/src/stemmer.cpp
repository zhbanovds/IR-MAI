#include "stemmer.h"

bool Stemmer::ends_with(const std::string& s, const std::string& suf) {
    if (s.size() < suf.size()) return false;
    return s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

void Stemmer::strip_suffix(std::string& s, const std::string& suf) {
    if (ends_with(s, suf)) {
        s.erase(s.size() - suf.size());
    }
}

std::string Stemmer::stem(const std::string& token) const {
    std::string s = token;
    if (s.size() <= 2) return s;

    static const char* suffixes[] = {
        "ization", "ational", "fulness", "ousness", "iveness", "tional",
        "biliti", "lessli", "entli", "ation", "alism", "aliti", "iviti",
        "enci", "anci", "abli", "izer", "ator", "alli", "bli",
        "ing", "edly", "edly", "ed", "ly", "es", "s"
    };

    for (const char* suf : suffixes) {
        std::string sufs(suf);
        if (s.size() > sufs.size() + 2 && ends_with(s, sufs)) {
            strip_suffix(s, sufs);
            break;
        }
    }

    if (s.size() < 2) s = token;
    return s;
}