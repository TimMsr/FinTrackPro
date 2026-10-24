//
// Created by Tim Mäser on 01/10/2025.
//

#ifndef FINTRACK_RECURRINGTRANSACTION_H
#define FINTRACK_RECURRINGTRANSACTION_H
#include "transaction.h"
#include "core/Ledger.h"


class RecurringTransaction : public Transaction {
private:
    std::string frequency;
public:
    RecurringTransaction(const std::string &id, const std::string &date, const Money &amount, Category category, const std::string freq,
                const std::string &note)
                : Transaction(id, date, amount, category, note), frequency(freq) {}

    // set how often the template should make a transfer
    void setFrequency(const std::string &freq);

    RecurringTransaction(const std::string &id, const std::string &date, const Money &amount, Category category,
                         const std::string &note);

    // quick accessor for the stored frequency string
    std::string getFrequency() const;

    // expand the template into concrete transactions for a month
    std::vector<Transaction> generateInstances(const std::string& month) const;


};


#endif //FINTRACK_RECURRINGTRANSACTION_H
