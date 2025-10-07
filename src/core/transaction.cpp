// Transaction implementation
// Mostly data holders with tiny helpers for summaries
#include "core/transaction.h"
#include "category.h"
#include "money.h"
#include <sstream>


// constructor formatted for easy readability
Transaction::Transaction(const std::string& id,
                         const std::string& date,
                         const Money& amount,
                         Category category,
                         const std::string& note)
    : ID(id), date(date), amount(amount), category(category), note(note) {}


// Simple getter methods
std::string Transaction::getID() const { return ID; }
std::string Transaction::getDate() const { return date; }
Money Transaction::getAmount() const { return amount; }
Category Transaction::getCategory() const { return category; }
std::string Transaction::getNote() const { return note; }

// setter methods (if needed)
void Transaction::setDate(const std::string& d) { date = d; }
void Transaction::setAmount(const Money& m) { amount = m; }
void Transaction::setCategory(Category c) { category = c; }
void Transaction::setNote(const std::string& n) { note = n; }

// simple comparison to return if its income or expense.
bool Transaction::isIncome() const {
    return amount.getMoney() > 0;
}

// Returns the YYYY-MM substring of the transaction date for grouping transactions by month.
std::string Transaction::monthKey() const {
    return date.substr(0,7);
}

// Builds a one-line string summary of the transaction (date, category, amount, note).
std::string Transaction::shortSummary() const {
    std::ostringstream oss;
    oss << date << " | "
        << categoryToString(category) << " | "
        << amount.toString() << " | "
        << note;
    return oss.str();
}

// stream operator prints the same short summary
std::ostream& operator<<(std::ostream& os, const Transaction& t) {
    os << t.shortSummary();
    return os;
}
