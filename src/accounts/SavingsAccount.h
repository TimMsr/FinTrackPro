//
// Created by Tim Mäser on 30/09/2025.
//

#ifndef FINTRACK_SAVINGSACCOUNT_H
#define FINTRACK_SAVINGSACCOUNT_H

#include "core/money.h"
#include <string>
#include "Account.h"


class SavingsAccount : public Account {
    public:
    double interestRate{0.05}; // annual rate, paid monthly

    explicit SavingsAccount(const std::string& name);

    void setInterestRate(double rate);
    double getInterestRate() const;

    void applyMonthlyInterest (const std::string& yyyy_mm);




};


#endif //FINTRACK_SAVINGSACCOUNT_H
