//
// Created by Tim Mäser on 30/09/2025.
//

#ifndef M2OEP_TMASER_CHECKINGACCOUNT_H
#define M2OEP_TMASER_CHECKINGACCOUNT_H

#include "core/money.h"
#include <string>
#include "Account.h"

class CheckingAccount : public Account {
private:
    double monthlyFee {0.0};
public:
    explicit CheckingAccount(const std::string& name, double fee = 0.0);
    // Override Account's monthly balance with fee applied
    Money balance(const std::string& yyyy_mm) const override;
};


#endif //M2OEP_TMASER_CHECKINGACCOUNT_H
