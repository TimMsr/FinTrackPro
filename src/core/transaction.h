// Transaction record
// Holds id, date, signed amount, category, and a short note.
#ifndef FINTRACK_TRANSACTION_H
#define FINTRACK_TRANSACTION_H
#include <string>
// Forward declaration for std::ostream
#include <iosfwd>

#include "category.h"
#include "money.h"

class Transaction {
private:
    std::string ID;
    std::string date;
    Money amount;
    Category category;
    std::string note;

public:
    // Build a transaction. amount is signed: positive for income, negative for expense.
    Transaction(const std::string &id, const std::string &date, const Money &amount, Category category,
                const std::string &note);


    // Getter methods:
    std::string getID() const;
    std::string getDate() const;
    Money getAmount() const;
    Category getCategory() const;
    std::string getNote() const;

    // Setter methods (light use only, prefer immutability when possible)
    void setDate(const std::string& date);
    void setAmount(const Money& amount);
    void setCategory(Category c);
    void setNote(const std::string& note);

    // simple helper method to make code more clean.
    bool isIncome() const;

    // returns YYYY-MM derived from the date string
    std::string monthKey() const;
    // short one line print for lists
    std::string shortSummary() const;

    // Overloaded stream insertion operator for easy printing of Transaction objects
    friend std::ostream& operator<<(std::ostream& os, const Transaction& t);

};


#endif //FINTRACK_TRANSACTION_H
