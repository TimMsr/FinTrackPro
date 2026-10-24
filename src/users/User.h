//
// Has-A relationship (one user has multiple accounts)
//

#ifndef FINTRACK_USER_H
#define FINTRACK_USER_H
#include <string>
#include "accounts/Account.h"
#include "core/money.h"


class User {
private:
    std::string name;
    std::vector<Account*> accounts; // Polymorphic: can hold Checking, Savings, CreditCard

public:

    User(const std::string& n) : name(n) {}

    // attach a new account pointer to this user
    void addAccount(Account* acc);

    // print a quick rundown of each owned account
    void listAccounts() const;
    // sum balances across accounts for the specified month
    Money getTotalNetWorth(const std::string& yyyy_mm) const;
    // expose raw account list when the UI needs it
   const std::vector<Account*>& getAccounts() const;

    // return the user's display name
    std::string getName() const;
};


#endif //FINTRACK_USER_H
