//
// Created by Tim Mäser on 01/10/2025.
//

#include "RecurringTransaction.h"


// Constructor forwards parameters to the base Transaction constructor
RecurringTransaction::RecurringTransaction(const std::string& id,
                         const std::string& date,
                         const Money& amount,
                         Category category,
                         const std::string& note)
    : Transaction(id, date, amount, category, note) {}



// build a batch of transactions based on the stored cadence
std::vector<Transaction> RecurringTransaction::generateInstances(const std::string &month) const {
    std::vector<Transaction> instances;
    // Simple example: if frequency is "monthly", just create one transaction in that month
    if (frequency == "monthly") {
        Transaction tx(getID() + "-R",
                       month + "-01",
                       getAmount(),
                       getCategory(),
                       getNote() + " (Recurring)");
        instances.push_back(tx);
    } else if (frequency == "weekly") {
        for (int day = 1; day <= 28; day += 7) {
            // pretend each week falls on Monday for simplicity
            Transaction tx(getID() + "-R",
                           month + "-01",
                           getAmount(),
                           getCategory(),
                           getNote() + " (Recurring Weekly)");
            instances.push_back(tx);
        }
    } else if (frequency == "daily") {
        for (int day = 1; day <= 28; day++) {
            // generate one entry per day up to 28 to keep month lengths consistent
            Transaction tx(getID() + "-R",
                           month + "-01",
                           getAmount(),
                           getCategory(),
                           getNote() + " (Recurring Daily)");
            instances.push_back(tx);
        }
    }
    return instances;
}

// update how often this template should repeat
void RecurringTransaction::setFrequency(const std::string &freq) {
    frequency = freq;
}
// expose the current frequency string
std::string RecurringTransaction::getFrequency() const {
    return frequency;
}
