// Utility validators
// date formats and amount parsing

#include "utility.h"
#include "core/Ledger.h"
#include <string>
#include <algorithm>

bool validDateFull(const std::string& date) {
    // expect YYYY-MM-DD, not checking calendar edge cases here since that is not needed.
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;

    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));

        if (year < 1900 || year > 2100) return false; // bounds to surpress typos
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
        return true;
    } catch (...) {
        return false;
    }
}
bool validDateMonth(const std::string& date) {
    // Day part not added hear, only focussed on month and year validation
    if (date.size() != 7 || date[4] != '-') return false;

    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        if (year > 2026) return false;
        if (month < 1 || month > 12) return false;
        return true;
    } catch (...) {
        return false;
    }
}

bool validAmount(const std::string& s) {
    // allow $ prefix, optional decimal point, max 2 decimals
    if (s.empty()) return false;

    size_t start = (s[0] == '$') ? 1 : 0;  // allow optional $
    int dots = 0; // dots counter

    for (size_t i = start; i < s.size(); i++) {
        if (s[i] == '.') {
            dots++;
            if (dots > 1) return false; // only one decimal allowed
        } else if (!isdigit(s[i])) {
            return false; // anything else invalid
        }
    }

    // if there is a decimal, ensure max 2 digits after it
    size_t dotPos = s.find('.');
    if (dotPos != std::string::npos) {
        if (s.size() - dotPos - 1 > 2) return false;
    }

    return true;
}
bool validCat(const std::string& s) {
    // case insensitive match against known category names
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return (lower == "food" ||
            lower == "rent" ||
            lower == "utilities" ||
            lower == "transport" ||
            lower == "income" ||
            lower == "other");
}
