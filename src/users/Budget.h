#ifndef M2OEP_TMASER_BUDGET_H
#define M2OEP_TMASER_BUDGET_H
#include <map>
#include "Category.h"
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

    // Compare spending vs budget
    Money variance(Category cat, const Money& spent) const;

};


#endif //M2OEP_TMASER_BUDGET_H
