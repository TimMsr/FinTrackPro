//
// Created by Tim Mäser on 30/09/2025.
//

#ifndef FINTRACK_CREDITACCOUNT_H
#define FINTRACK_CREDITACCOUNT_H

#include "Account.h"
#include "core/money.h"

class CreditAccount : public Account {
private:
    Money creditLimit{0};
public:
    explicit CreditAccount(const std::string& name, const Money& limit = Money(0));

    void setCreditLimit(const Money& limit);
    Money getCreditLimit() const;
};


#endif //FINTRACK_CREDITACCOUNT_H
