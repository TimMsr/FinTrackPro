// Category definitions
// Simple enum for tagging transactions with broad buckets
// keep names stable to avoid brittle string matches

#ifndef FINTRACK_CATEGORY_H
#define FINTRACK_CATEGORY_H

#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>

enum class Category {
    Food,
    Rent,
    Utilities,
    Transport,
    Income,
    Transfer,
    Other
};

// convert Category to title case string
std::string categoryToString(Category c);
// parse string to Category, case insensitive, throws on unknown
Category stringToCategory(const std::string& str);

#endif

#endif //FINTRACK_CATEGORY_H
