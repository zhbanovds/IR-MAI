#include "bool_ops.h"

std::vector<std::uint32_t> op_and(const std::vector<std::uint32_t>& a, const std::vector<std::uint32_t>& b) {
    std::vector<std::uint32_t> out;
    out.reserve((a.size() < b.size()) ? a.size() : b.size());

    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            out.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return out;
}

std::vector<std::uint32_t> op_or(const std::vector<std::uint32_t>& a, const std::vector<std::uint32_t>& b) {
    std::vector<std::uint32_t> out;
    out.reserve(a.size() + b.size());

    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            out.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            out.push_back(a[i++]);
        } else {
            out.push_back(b[j++]);
        }
    }
    while (i < a.size()) out.push_back(a[i++]);
    while (j < b.size()) out.push_back(b[j++]);

    return out;
}

std::vector<std::uint32_t> op_not(const std::vector<std::uint32_t>& a, const std::vector<std::uint32_t>& universe) {
    std::vector<std::uint32_t> out;
    out.reserve(universe.size());

    std::size_t i = 0, j = 0;
    while (i < universe.size()) {
        if (j >= a.size()) {
            out.push_back(universe[i++]);
            continue;
        }
        if (universe[i] == a[j]) {
            ++i; ++j;
        } else if (universe[i] < a[j]) {
            out.push_back(universe[i++]);
        } else {
            ++j;
        }
    }
    return out;
}