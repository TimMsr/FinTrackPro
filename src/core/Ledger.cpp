// Ledger implementation
// Simple storage with linear scans. fine for small datasets

#include "core/Ledger.h"

// Adds a new transaction to the ledger
void Ledger::add(const Transaction& t) {
    items.push_back(t);
}

// Computes sum of transactions by month
Money Ledger::sumByMonth(const std::string& yyyy_mm) const {
    Money sum(0);
    for (const Transaction& t : items) {
        if (t.monthKey()== yyyy_mm) {
            sum = sum + t.getAmount();
        }
    }
    return sum;
}

// Computes sum of transactions by month and category
Money Ledger::sumByCategory(const std::string& yyyy_mm, Category cat) const {
    Money sum(0);
    for (const Transaction& t : items) {
        if (t.getCategory() == cat && t.monthKey()== yyyy_mm) {
            sum = sum + t.getAmount();
        }
    }
    return sum;
}

// Finds all transactions where the note contains a keyword
std::vector<Transaction> Ledger::search(const std::string &keyword) const {
    std::vector<Transaction> result;
    for (const Transaction& t : items) {
        if (t.getNote().find(keyword) != std::string::npos) {
            result.push_back(t);
        }
    }
    return result;
}

// Lists all transactions that fall within a specific month
std::vector<Transaction> Ledger::listByMonth(const std::string &yyyy_mm) const {
    std::vector<Transaction> result;
    for (const Transaction& t : items) {
        if (t.monthKey()== yyyy_mm) {
            result.push_back(t);
        }
    }
    return result;
}

void Ledger::clear() {
    items.clear();
}
