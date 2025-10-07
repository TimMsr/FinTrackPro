//
// Created by Tim Mäser on 30/09/2025.
//

#include "CheckingAccount.h"
#include <cmath>

// store optional monthly service fee so balance reflects it later
CheckingAccount::CheckingAccount(const std::string& name, double fee)
    : Account(name), monthlyFee(fee) {}

// override so we subtract the fee after the base calculationn
Money CheckingAccount::balance(const std::string& yyyy_mm) const {
    Money base = Account::balance(yyyy_mm);
    long long feeCents = static_cast<long long>(std::llround(monthlyFee * 100.0));
    return base - Money(feeCents);
}
