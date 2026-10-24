//
// Created by Tim Mäser on 01/10/2025.
//

#ifndef FINTRACK_TRANSFERTRANSACTION_H
#define FINTRACK_TRANSFERTRANSACTION_H
#include "transaction.h"
#include "accounts/Account.h"
#include "accounts/CreditAccount.h"
#include "accounts/SavingsAccount.h"
#include "accounts/CheckingAccount.h"



class TransferTransaction: public Transaction {
public:
    using Transaction::Transaction;

    // move money between two accounts while logging mirror entries
    // returns false when the combination of accounts is not allowed
    bool execute(Account& from, Account& to) const;

};


#endif //FINTRACK_TRANSFERTRANSACTION_H
