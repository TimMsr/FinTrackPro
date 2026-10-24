//
// Created by Tim Mäser on 01/10/2025.
//
#include "TransferTransaction.h"

bool TransferTransaction::execute(Account& from, Account& to) const {
    // block the one transfer combination we do not support
    if (dynamic_cast<CreditAccount*>(&from) && dynamic_cast<SavingsAccount*>(&to)) {
        return false;
    }
    // log matching entries on both sides: a debit on the source, a credit on the destination
    Money amount = getAmount();
    from.addTransaction(Transaction(getID() + "-OUT", getDate(), Money(0) - amount, Category::Transfer,
                                    "Transfer to " + to.getName()));
    to.addTransaction(Transaction(getID() + "-IN", getDate(), amount, Category::Transfer,
                                  "Transfer from " + from.getName()));
    return true;
}
