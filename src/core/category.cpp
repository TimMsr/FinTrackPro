// Category helpers
// provides string conversion in both directions
#include "core/Category.h"
#include <algorithm>
#include <stdexcept>

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string categoryToString(Category c) {
    switch (c) {
        case Category::Food: return "Food";
        case Category::Rent: return "Rent";
        case Category::Utilities: return "Utilities";
        case Category::Transport: return "Transport";
        case Category::Income: return "Income";
        case Category::Transfer: return "Transfer";
        case Category::Other: return "Other";
    }
    return "Other";
}

Category stringToCategory(const std::string& str) {
    // compares input to available categories and returns it.
    std::string s = toLower(str);
    if (s == "food") return Category::Food;
    if (s == "rent") return Category::Rent;
    if (s == "utilities") return Category::Utilities;
    if (s == "transport") return Category::Transport;
    if (s == "transfer") return Category::Transfer;
    if (s == "income") return Category::Income;
    if (s == "other") return Category::Other;
    throw std::invalid_argument("Unknown category: " + str);
}
