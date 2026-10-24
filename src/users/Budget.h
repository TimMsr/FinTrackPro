#ifndef FINTRACK_BUDGET_H
#define FINTRACK_BUDGET_H
#include <map>
#include "category.h"
#include "money.h"

class Budget {
private:
    std::map<Category, Money> limits; // Category -> Limit

public:
    Budget() = default;

    // store a limit for the given category
    void setLimit(Category cat, const Money& amount);
    // fetch the saved limit (defaults to zero if unset)
    Money getLimit(Category cat);

    // every category that has a limit set
    const std::map<Category, Money>& getLimits() const { return limits; }

    // Compare spending vs budget
    Money variance(Category cat, const Money& spent) const;

};


#endif //FINTRACK_BUDGET_H
