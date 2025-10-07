//
// Created by Tim Mäser on 30/09/2025.
//

#ifndef M2OEP_TMASER_SAVINGSACCOUNT_H
#define M2OEP_TMASER_SAVINGSACCOUNT_H

#include "core/money.h"
#include <string>
#include "Account.h"


class SavingsAccount : public Account {
    public:
    double interestRate{0.05};

    explicit SavingsAccount(const std::string& name);

    Money balance(const std::string& yyyy_mm) const override;

    void setInterestRate(double rate);
    double getInterestRate() const;

    void applyMonthlyInterest (const std::string& yyyy_mm);




};


#endif //M2OEP_TMASER_SAVINGSACCOUNT_H
