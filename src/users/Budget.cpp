

#include "Budget.h"


// remember the desired cap for this category
void Budget::setLimit(Category cat, const Money &amount) {
    limits[cat] = amount;
}

// return stored limit (default-constructs if not present)
Money Budget::getLimit(const Category cat) {
    return limits[cat];
}

// calculate how far under/over the budget the spending is
Money Budget::variance(Category cat, const Money &spent) const {
    auto it = limits.find(cat);
    if (it != limits.end()) {
        return it->second - spent;
    }
    // If no limit set for this category, treat as "overspent by spent"
    return Money(0) - spent;
}
