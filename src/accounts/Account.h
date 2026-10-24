//
// Created by Tim Mäser on 30/09/2025.
//

#ifndef FINTRACK_ACCOUNT_H
#define FINTRACK_ACCOUNT_H
#include "accounts/Account.h"
#include <string>
#include <sstream>
#include "core/Ledger.h"

class Account {
protected:
    std::string name;    // Account name ("UVM Checking", "Visa CC")
    Ledger ledger;     // Has-A Ledger
public:
    Account(const std::string& name);
    virtual ~Account() = default;

    std::string getName() const;

    // Core methods
    virtual Money balance(const std::string& yyyy_mm) const;  // monthly balance
    void addTransaction(const Transaction& tx);
    const Ledger& getLedger() const;
    void clearTransactions();


};


#endif //FINTRACK_ACCOUNT_H
