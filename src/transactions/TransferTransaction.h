//
// Created by Tim Mäser on 01/10/2025.
//

#ifndef M2OEP_TMASER_TRANSFERTRANSACTION_H
#define M2OEP_TMASER_TRANSFERTRANSACTION_H
#include "transaction.h"
#include "accounts/Account.h"
#include "accounts/CreditAccount.h"
#include "accounts/SavingsAccount.h"
#include "accounts/CheckingAccount.h"



class TransferTransaction: public Transaction {
public:
    // move money between two accounts while logging mirror entries
    void execute(Account& from, Account& to, const Money& amount);

};


#endif //M2OEP_TMASER_TRANSFERTRANSACTION_H
