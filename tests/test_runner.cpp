#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>

#include "users/Budget.h"
#include "users/User.h"
#include "accounts/CheckingAccount.h"
#include "accounts/SavingsAccount.h"
#include "transactions/RecurringTransaction.h"
#include "core/transaction.h"
#include "core/category.h"
#include "core/money.h"

//My Test cases
// I only implemented around 11 since testing every method that I have would make it impossible.

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

// Checks if the savings account applies the interest correctly
void testSavingsAccountAppliesInterest() {
    SavingsAccount account("High Yield");
    account.addTransaction(Transaction("SAV-1", "2025-02-01", moneyFrom("100.00"), Category::Income, "Seed"));
    Money result = account.balance("2025-02");
    expect(result == moneyFrom("105.00"), "Balance should include monthly interest");
}

// Tests if a recurring monthly transaction creates the transaction (and only makes it once a month)
void testRecurringTransactionMonthlySingleEntry() {
    RecurringTransaction rent("Rent", "2025-01-01", moneyFrom("900.00"), Category::Rent, "monthly", "Apartment");
    auto items = rent.generateInstances("2025-02");

    expect(items.size() == 1, "Monthly recurring should create one entry");
    expect(items.front().getID() == "Rent-R", "Generated ID should suffix with -R");
}

// Tests if a recurring weekly transaction creates 4 entries correctly.
void testRecurringTransactionWeeklyFourEntries() {
    RecurringTransaction gym("Gym", "2025-01-01", moneyFrom("20.00"), Category::Other, "weekly", "Gym membership");
    auto items = gym.generateInstances("2025-02");
    expect(items.size() == 4, "Weekly recurring should create four entries");
}

// Tests if a recurring daily transaction creates 28 entries correctly (since we currently operate on 28 days).
void testRecurringTransactionDailyTwentyEightEntries() {
    RecurringTransaction coffee("Coffee", "2025-01-01", moneyFrom("4.00"), Category::Food, "daily", "Daily coffee");
    auto items = coffee.generateInstances("2025-02");
    expect(items.size() == 28, "Daily recurring should create twenty-eight entries (simplified month)");
}

// Tests if recurring transaction getter and setter match
void testRecurringTransactionSetFrequencyGetter() {
    RecurringTransaction lunch("Lunch", "2025-01-01", moneyFrom("8.50"), Category::Food, "monthly", "Lunch");
    lunch.setFrequency("weekly");
    expect(lunch.getFrequency() == "weekly", "Frequency setter/getter should match");
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
        {"Savings account applies interest", testSavingsAccountAppliesInterest},
        {"Recurring monthly generates single entry", testRecurringTransactionMonthlySingleEntry},
        {"Recurring weekly generates four entries", testRecurringTransactionWeeklyFourEntries},
        {"Recurring daily generates twenty-eight entries", testRecurringTransactionDailyTwentyEightEntries},
        {"Recurring set/get frequency", testRecurringTransactionSetFrequencyGetter}
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
