#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cstdio>

#include "users/Budget.h"
#include "users/User.h"
#include "accounts/CheckingAccount.h"
#include "accounts/SavingsAccount.h"
#include "transactions/RecurringTransaction.h"
#include "transactions/TransferTransaction.h"
#include "accounts/CreditAccount.h"
#include "io/CsvHandler.h"
#include "core/transaction.h"
#include "core/category.h"
#include "core/money.h"

// Unit tests for the FinTrack Pro core library

struct TestCase {
    const char* name;
    std::function<void()> fn;
};

void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Money moneyFrom(const std::string& value) {
    return Money::fromString(value);
}


// tests simple setting budget limit and getting the limit
void testBudgetSetAndGetLimit() {
    Budget budget;
    budget.setLimit(Category::Food, moneyFrom("150.00"));
    expect(budget.getLimit(Category::Food) == moneyFrom("150.00"), "Budget should return stored limit");
}

// Test the set limit and seeing if the variance is correctly calculated and under the limit
void testBudgetVarianceUnderLimit() {
    Budget budget;
    budget.setLimit(Category::Rent, moneyFrom("500.00"));
    Money variance = budget.variance(Category::Rent, moneyFrom("320.00"));
    expect(variance == moneyFrom("180.00"), "Variance should be limit minus spent when under budget");
}

// Test the set limit and seeing if the variance is correctly calculate and gives the variance signifying overspendature
void testBudgetVarianceOverspend() {
    Budget budget;
    budget.setLimit(Category::Utilities, moneyFrom("200.00"));
    Money variance = budget.variance(Category::Utilities, moneyFrom("240.00"));
    expect(variance == moneyFrom("-40.00"), "Variance should be negative when overspending");
}


// Test checks if you can create 2 of the same accounts
void testUserAddAccountPreventsDuplicate() {
    User user("Tester");
    CheckingAccount* checking = new CheckingAccount("Everyday", 0.0);
    user.addAccount(checking);
    user.addAccount(checking); // duplicate pointer should be denied

    expect(user.getAccounts().size() == 1, "User already exists!");
    delete checking;
}

// Tests the net worth of a specified user accross multiple accounts
void testUserTotalNetWorthAcrossAccounts() {
    User user("FinFan");
    CheckingAccount* checking = new CheckingAccount("Check", 0.0);
    SavingsAccount* savings = new SavingsAccount("Save");
    savings->setInterestRate(0.0); // keep math simple for this test

    checking->addTransaction(Transaction("C1", "2025-01-10", moneyFrom("200.00"), Category::Income, "Paycheck"));
    savings->addTransaction(Transaction("S1", "2025-01-12", moneyFrom("150.00"), Category::Income, "Bonus for great employment"));

    user.addAccount(checking);
    user.addAccount(savings);

    Money total = user.getTotalNetWorth("2025-01");
    expect(total == moneyFrom("350.00"), "Net worth should sum balances from all accounts");

    delete checking;
    delete savings;
}


// Tests if checking account applies the monthly fee
void testCheckingAccountAppliesMonthlyFee() {
    CheckingAccount account("Campus Checking", 5.00);
    account.addTransaction(Transaction("CHK-1", "2025-03-05", moneyFrom("100.00"), Category::Income, "Deposit"));
    Money result = account.balance("2025-03");
    expect(result == moneyFrom("95.00"), "Balance should subtract monthly fee once");

}

// Checks that savings interest is paid once, at 1/12 of the annual rate
void testSavingsAccountAppliesMonthlyInterest() {
    SavingsAccount account("High Yield");
    account.setInterestRate(0.06);
    account.addTransaction(Transaction("SAV-1", "2025-02-01", moneyFrom("1000.00"), Category::Income, "Seed"));
    expect(account.balance("2025-02") == moneyFrom("1000.00"), "Reading the balance must not add interest");
    account.applyMonthlyInterest("2025-02");
    expect(account.balance("2025-02") == moneyFrom("1005.00"), "One month of 6% APR on $1000 is $5");
}

// Tests if a recurring monthly transaction creates the transaction (and only makes it once a month)
void testRecurringTransactionMonthlySingleEntry() {
    RecurringTransaction rent("Rent", "2025-01-05", moneyFrom("900.00"), Category::Rent, "monthly", "Apartment");
    auto items = rent.generateInstances("2025-02");

    expect(items.size() == 1, "Monthly recurring should create one entry");
    expect(items.front().getID() == "Rent-R", "Generated ID should suffix with -R");
    expect(items.front().getDate() == "2025-02-05", "Monthly entry should keep the start date's day");
}

// Monthly entries on the 31st fall back to the last day of shorter months
void testRecurringTransactionClampsToMonthEnd() {
    RecurringTransaction bill("Bill", "2025-01-31", moneyFrom("50.00"), Category::Utilities, "monthly", "Power");
    auto items = bill.generateInstances("2024-02");
    expect(items.front().getDate() == "2024-02-29", "Leap-year February should clamp to the 29th");
}

// Tests if a recurring weekly transaction lands on distinct days a week apart
void testRecurringTransactionWeeklySpreadsDates() {
    RecurringTransaction gym("Gym", "2025-01-01", moneyFrom("20.00"), Category::Other, "weekly", "Gym membership");
    auto items = gym.generateInstances("2025-02");
    expect(items.size() == 4, "February 2025 has four weekly slots");
    expect(items[1].getDate() == "2025-02-08", "Second entry should be seven days after the first");
}

// Daily recurring follows the real month length
void testRecurringTransactionDailyMatchesCalendar() {
    RecurringTransaction coffee("Coffee", "2025-01-01", moneyFrom("4.00"), Category::Food, "daily", "Daily coffee");
    expect(coffee.generateInstances("2025-02").size() == 28, "February 2025 has 28 days");
    expect(coffee.generateInstances("2025-03").size() == 31, "March has 31 days");
}

// Tests if recurring transaction getter and setter match
void testRecurringTransactionSetFrequencyGetter() {
    RecurringTransaction lunch("Lunch", "2025-01-01", moneyFrom("8.50"), Category::Food, "monthly", "Lunch");
    lunch.setFrequency("weekly");
    expect(lunch.getFrequency() == "weekly", "Frequency setter/getter should match");
}

// Negative amounts keep their sign when formatted and parse back to the same value
void testMoneyNegativeRoundTrip() {
    Money small(-50);
    expect(small.toString() == "-$0.50", "Sub-dollar negatives must keep the minus sign");
    expect(Money::fromString(small.toString()) == small, "Formatted money should parse back");
}

// A transfer debits the source and credits the destination, so net worth is unchanged
void testTransferConservesMoney() {
    CheckingAccount checking("Checking");
    SavingsAccount savings("Savings");
    checking.addTransaction(Transaction("C1", "2025-01-01", moneyFrom("500.00"), Category::Income, "Pay"));
    TransferTransaction transfer("T1", "2025-01-02", moneyFrom("200.00"), Category::Transfer, "");
    expect(transfer.execute(checking, savings), "Checking to savings should be allowed");
    expect(checking.getLedger().total() == moneyFrom("300.00"), "Source should be debited");
    expect(savings.getLedger().total() == moneyFrom("200.00"), "Destination should be credited");
}

// Credit to savings transfers are rejected without touching either ledger
void testTransferBlocksCreditToSavings() {
    CreditAccount card("Card");
    SavingsAccount savings("Savings");
    TransferTransaction transfer("T2", "2025-01-02", moneyFrom("50.00"), Category::Transfer, "");
    expect(!transfer.execute(card, savings), "Credit to savings should be refused");
    expect(card.getLedger().getAll().empty() && savings.getLedger().getAll().empty(), "No entries on refusal");
}

// Notes containing commas survive a write/read cycle
void testCsvRoundTripKeepsCommasInNotes() {
    const std::string file = "fintrack_test_ledger.csv";
    Ledger ledger;
    ledger.add(Transaction("A1", "2025-04-03", moneyFrom("-12.50"), Category::Food, "Lunch, with team"));
    CsvHandler csv;
    csv.writeLedger(file, ledger, "Checking", "checking");
    Ledger loaded;
    csv.readCSV(file, loaded);
    std::remove(file.c_str());
    expect(loaded.getAll().size() == 1, "One row should be read back");
    expect(loaded.getAll()[0].getNote() == "Lunch, with team", "Comma inside note should be preserved");
    expect(loaded.getAll()[0].getAmount() == moneyFrom("-12.50"), "Negative amount should round-trip");
}

// Budget limits persist to disk
void testBudgetRoundTrip() {
    const std::string file = "fintrack_test_budget.csv";
    Budget budget;
    budget.setLimit(Category::Food, moneyFrom("400.00"));
    budget.setLimit(Category::Transport, moneyFrom("120.00"));
    CsvHandler csv;
    csv.writeBudget(file, budget);
    Budget loaded;
    csv.readBudget(file, loaded);
    std::remove(file.c_str());
    expect(loaded.getLimit(Category::Food) == moneyFrom("400.00"), "Food limit should be restored");
    expect(loaded.getLimit(Category::Transport) == moneyFrom("120.00"), "Transport limit should be restored");
}


// Main method below initializing it.
int main() {
    std::vector<TestCase> tests = {
        {"Budget set and get limit", testBudgetSetAndGetLimit},
        {"Budget variance under limit", testBudgetVarianceUnderLimit},
        {"Budget variance overspend", testBudgetVarianceOverspend},
        {"User add account prevents duplicate", testUserAddAccountPreventsDuplicate},
        {"User total net worth across accounts", testUserTotalNetWorthAcrossAccounts},
        {"Checking account applies monthly fee", testCheckingAccountAppliesMonthlyFee},
        {"Savings interest applied once per month", testSavingsAccountAppliesMonthlyInterest},
        {"Recurring monthly generates single entry", testRecurringTransactionMonthlySingleEntry},
        {"Recurring monthly clamps to month end", testRecurringTransactionClampsToMonthEnd},
        {"Recurring weekly spreads dates", testRecurringTransactionWeeklySpreadsDates},
        {"Recurring daily matches calendar", testRecurringTransactionDailyMatchesCalendar},
        {"Recurring set/get frequency", testRecurringTransactionSetFrequencyGetter},
        {"Money negative round trip", testMoneyNegativeRoundTrip},
        {"Transfer conserves money", testTransferConservesMoney},
        {"Transfer blocks credit to savings", testTransferBlocksCreditToSavings},
        {"CSV round trip keeps commas in notes", testCsvRoundTripKeepsCommasInNotes},
        {"Budget round trip", testBudgetRoundTrip}
    };

    int failures = 0;
    for (const auto& test : tests) {
        try {
            test.fn();
            std::cout << "[PASS] " << test.name << "\n";
        } catch (const std::exception& ex) {
            ++failures;
            std::cout << "[FAIL] " << test.name << " -- " << ex.what() << "\n";
        }
    }

    std::cout << "\n" << (tests.size() - failures) << "/" << tests.size() << " tests passed" << std::endl;
    return failures == 0 ? 0 : 1;
}
