// Category definitions
// Simple enum for tagging transactions with broad buckets
// keep names stable to avoid brittle string matches

#ifndef M1OEP_TMASER_CATEGORY_H
#define M1OEP_TMASER_CATEGORY_H

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

#endif //M1OEP_TMASER_CATEGORY_H
