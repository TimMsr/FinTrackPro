//
// Created by Tim Mäser on 01/10/2025.
//
#include <iostream>
#include "TransferTransaction.h"

void TransferTransaction::execute(Account& from, Account& to, const Money& amount) {
    // block the one transfer combination we do not support
    if (dynamic_cast<CreditAccount*>(&from) && dynamic_cast<SavingsAccount*>(&to)) {
        std::cout << "Error: Cannot transfer directly from Credit to Savings." << std::endl;
        return;
    }
    // log matching entries on both sides of the transfer
    from.addTransaction(Transaction("X", getDate(), amount, Category::Transfer, "Transfer Out"));
    to.addTransaction(Transaction("X", getDate(), amount, Category::Transfer, "Transfer In"));

}
