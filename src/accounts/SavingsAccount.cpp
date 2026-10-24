//
// Created by Tim Mäser on 30/09/2025.
//

#include "SavingsAccount.h"
#include "core/money.h"
#include "core/Ledger.h"

// basic savings ctor keeps default interest rate
SavingsAccount::SavingsAccount(const std::string& name)
    : Account(name) {}

// allow tweaking rate when user configures the account
void SavingsAccount::setInterestRate(double rate) {
    interestRate = rate;
}

double SavingsAccount::getInterestRate() const {
    return interestRate;
}


// drop a synthetic interest transaction into the ledger for that month
void SavingsAccount::applyMonthlyInterest(const std::string& yyyy_mm) {
    // interest accrues on the running balance at 1/12 of the annual rate
    Money interest = ledger.total() * (interestRate / 12.0);
    Transaction t("INT-" + yyyy_mm, yyyy_mm + "-28", interest, Category::Income, "Monthly Interest");
    ledger.add(t);
}
