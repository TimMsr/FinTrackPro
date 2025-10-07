//
// Created by Tim Mäser on 30/09/2025.
//

#include "User.h"

#include <iostream>


// keep list from double-storing the same account pointer
void User::addAccount(Account* acc) {
    for (Account* acc1 : accounts) {
        if (acc == acc1) {
            std::cout << "User already exists!" << std::endl;
            return;
        }
    }
    accounts.push_back(acc);
}
// simple console summary of each account
void User::listAccounts() const {
    for (Account* acc : accounts) {
        Money bal = acc->balance("2025-02");
        std::cout << acc->getName()
                  << " | Balance: "
                  << bal.toString() << std::endl;

    }
}
// add up balances across every linked account for a month
Money User::getTotalNetWorth(const std::string& yyyy_mm) const {
    Money total(0);
    for (Account* acc : accounts) {
        total = total + acc->balance(yyyy_mm);
    }
    return total;
}

// expose list when the UI needs to iterate
const std::vector<Account *> &User::getAccounts() const {
    return accounts;
}

// quick accessor for user name
std::string User::getName() const {
    return name;
}
