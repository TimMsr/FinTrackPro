// Ledger interface
// In memory list of transactions with small query helpers
// Some methods are not used yet in this module, since i did not have time to fully incorporate them.
// For example, sumByMonth is not used yet but will be used for future module.

#ifndef M1OEP_TMASER_LEDGER_H
#define M1OEP_TMASER_LEDGER_H

#include "transaction.h"
#include <vector>
#include <string>


class Ledger {
private:
    std::vector<Transaction> items;

public:
    // add a transaction to the in memory list
    void add(const Transaction& t);

    // list transactions for a given month (YYYY-MM)
    std::vector<Transaction> listByMonth(const std::string& yyyy_mm) const;

    // sum total balance for a given month
    Money sumByMonth(const std::string& yyyy_mm) const;

    // sum by a category for a given month
    Money sumByCategory(const std::string& yyyy_mm, Category cat) const;

    // search for a keyword in notes
    std::vector<Transaction> search(const std::string& keyword) const;

    // get all transactions
    const std::vector<Transaction>& getAll() const { return items; }

    // clear all transactions (used when reloading from CSV)
    void clear();
};


#endif //M1OEP_TMASER_LEDGER_H
