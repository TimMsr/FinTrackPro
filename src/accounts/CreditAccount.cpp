//
// Created by Tim Mäser on 30/09/2025.
//

#include "CreditAccount.h"

// ctor tracks starting credit limit alongside base account info
CreditAccount::CreditAccount(const std::string& name, const Money& limit)
    : Account(name), creditLimit(limit) {}

// update the ceiling if the user tweaks their credit line
void CreditAccount::setCreditLimit(const Money& limit) {
    creditLimit = limit;
}

// expose limit for UI summaries
Money CreditAccount::getCreditLimit() const {
    return creditLimit;
}
