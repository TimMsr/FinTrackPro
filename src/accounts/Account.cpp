//
// Created by Tim Mäser on 30/09/2025.
//

#include "Account.h"

// basic ctor so each account can stash its label
Account::Account(const std::string &name) {
    this->name = name;
}

// returns the display name for menus
std::string Account::getName() const {
    return name;
}

// sums the ledger for a given month (shared by derived accounts)
Money Account::balance(const std::string &yyyy_mm) const {
    return ledger.sumByMonth(yyyy_mm);
}

// put a transaction straight into the account ledger
void Account::addTransaction(const Transaction& tx) {
    ledger.add(tx);
}

const Ledger& Account::getLedger() const {
    return ledger;
}

// wipe transactions before re-importing from disk
void Account::clearTransactions() {
    ledger.clear();
}
