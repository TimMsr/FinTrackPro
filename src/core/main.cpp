// FinTrack Pro terminal entrypoint
// Console workflow for creating accounts, importing data, and managing activity.
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "utility.h"
#include "accounts/Account.h"
#include "accounts/CheckingAccount.h"
#include "accounts/CreditAccount.h"
#include "accounts/SavingsAccount.h"
#include "core/Ledger.h"
#include "core/category.h"
#include "core/money.h"
#include "core/transaction.h"
#include "io/CsvHandler.h"
#include "transactions/RecurringTransaction.h"
#include "users/Budget.h"

struct AccountRecord {
    std::unique_ptr<Account> account;
    Budget budget;
    std::string accountType;
    std::string csvFile;
};

// Helper methods from main, goal is to move these to utility class at some point


std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string lowerCopy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

std::string promptNonEmpty(const std::string& prompt) {
    while (true) {
        std::string value = readLine(prompt);
        if (!value.empty()) {
            return value;
        }
        // refuse to accept a blank line
        std::cout << "Input cannot be empty. Try again.\n";
    }
}

int promptIntInRange(const std::string& prompt, int min, int max) {
    while (true) {
        std::string text = readLine(prompt);
        if (text.empty()) {
            std::cout << "Please enter a number between " << min << " and " << max << ".\n";
            continue;
        }
        try {
            size_t idx = 0;
            int value = std::stoi(text, &idx);
            if (idx == text.size() && value >= min && value <= max) {
                // valid number and sits in the requested bounds
                return value;
            }
        } catch (const std::exception&) {
            // fall through to retry message
        }
        std::cout << "Please enter a number between " << min << " and " << max << ".\n";
    }
}

std::string promptFullDate(const std::string& prompt) {
    while (true) {
        std::string date = readLine(prompt);
        if (validDateFull(date)) {
            return date;
        }
        std::cout << "Date must be in YYYY-MM-DD format.\n";
    }
}

std::string promptMonthKey(const std::string& prompt) {
    while (true) {
        std::string month = readLine(prompt);
        if (validDateMonth(month)) {
            return month;
        }
        std::cout << "Month must be in YYYY-MM format.\n";
    }
}

Money promptMoney(const std::string& prompt) {
    while (true) {
        std::string input = readLine(prompt);
        if (!validAmount(input)) {
            std::cout << "Invalid amount. Example: 12.34 or $12.34.\n";
            continue;
        }
        try {
            // let Money handle conversion once formatting passes
            return Money::fromString(input);
        } catch (const std::exception&) {
            std::cout << "Unable to parse amount. Try again.\n";
        }
    }
}

Category promptCategory(const std::string& prompt) {
    while (true) {
        std::string input = readLine(prompt);
        if (!validCat(input)) {
            std::cout << "Invalid category. Options: Food, Rent, Utilities, Transport, Income, Transfer, Other.\n";
            continue;
        }
        try {
            return stringToCategory(input);
        } catch (const std::exception&) {
            std::cout << "Unable to parse category. Try again.\n";
        }
    }
}

std::string promptFrequency() {
    while (true) {
        std::string freq = lowerCopy(promptNonEmpty("Frequency (daily/weekly/monthly): "));
        if (freq == "daily" || freq == "weekly" || freq == "monthly") {
            return freq;
        }
        std::cout << "Supported frequencies are daily, weekly, or monthly.\n";
    }
}

std::string promptAccountType() {
    while (true) {
        std::string type = lowerCopy(promptNonEmpty("Account type (credit/current/savings): "));
        if (type == "checking") {
            type = "current";
        }
        if (type == "credit" || type == "current" || type == "savings") {
            return type;
        }
        std::cout << "Valid options are credit, current, or savings.\n";
    }
}

// This method shows the account overview (key values that the user wants to see when he runs the code)
// Things such as which account, transactions, current balance and most recent transactions
void showAccountOverview(const Account& account) {
    const Ledger& ledger = account.getLedger();
    std::vector<Transaction> transactions = ledger.getAll();
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        // show most recent entries first in the overview list
        return a.getDate() > b.getDate();
    });

    Money balance(0);
    for (const auto& tx : ledger.getAll()) {
        // signed amounts let income and expenses offset properly
        balance = balance + tx.getAmount();
    }

    std::cout << "\n----- Account Overview -----\n";
    std::cout << "Account: " << account.getName() << "\n";
    std::cout << "Transactions recorded: " << transactions.size() << "\n";
    std::cout << "Current balance: " << balance.toString() << "\n";
    std::cout << "Most recent transactions:\n";
    if (transactions.empty()) {
        std::cout << "  (none)\n";
        return;
    }
    //Only output the most recent 5 transaction
    size_t limit = std::min<std::size_t>(5, transactions.size());
    for (size_t i = 0; i < limit; ++i) {
        std::cout << "  " << transactions[i] << "\n";
    }
}

std::vector<Transaction> collectTransfers(const Account& account) {
    std::vector<Transaction> transfers;
    for (const auto& tx : account.getLedger().getAll()) {
        if (tx.getCategory() == Category::Transfer) {
            // only keep transactions tagged as transfers
            transfers.push_back(tx);
        }
    }
    return transfers;
}

// Searches for transfers on a specific account and outputs the related message
//
void searchTransfers(const Account& account) {
    auto transfers = collectTransfers(account);
    if (transfers.empty()) {
        std::cout << "No transfer transactions for this account.\n";
        return;
    }

    std::string keyword = lowerCopy(readLine("Keyword to search (blank for all): "));
    std::vector<Transaction> matches;
    for (const auto& tx : transfers) {
        if (keyword.empty()) {
            // blank search prints every transfer
            matches.push_back(tx);
            continue;
        }
        std::string note = lowerCopy(tx.getNote());
        std::string id = lowerCopy(tx.getID());
        if (note.find(keyword) != std::string::npos || id.find(keyword) != std::string::npos) {
            // match either on note text or id
            matches.push_back(tx);
        }
    }

    std::cout << "Found " << matches.size() << " transfer(s).\n";
    for (const auto& tx : matches) {
        std::cout << "  " << tx << "\n";
    }
}

Money absoluteMoney(const Money& value) {
    long long cents = value.getMoney();
    return Money(cents < 0 ? -cents : cents);
}

std::string accountTypeForRecord(const AccountRecord& record) {
    if (!record.accountType.empty()) {
        return lowerCopy(record.accountType);
    }
    // fall back to RTTI so we still tag CSV rows with a sensible type
    if (dynamic_cast<const CreditAccount*>(record.account.get())) {
        return "credit";
    }
    if (dynamic_cast<const SavingsAccount*>(record.account.get())) {
        return "savings";
    }
    if (dynamic_cast<const CheckingAccount*>(record.account.get())) {
        return "current";
    }
    return "current";
}

void addTransactionAndPersist(AccountRecord& record, const Transaction& tx, const CsvHandler& csvHandler) {
    // update in-memory ledger first so UI reflects the change immediately
    record.account->addTransaction(tx);
    std::string effectiveType = accountTypeForRecord(record);
    record.accountType = effectiveType;
    if (record.csvFile.empty()) {
        // default CSV name mirrors the account label the first time we save
        record.csvFile = record.account->getName() + ".csv";
    }
    // keep on-disk CSV in sync with the in-memory ledger
    csvHandler.appendTransaction(record.csvFile, tx, record.account->getName(), effectiveType);
}

// Lists the transactions based on inputed month
// Outputs it straight to console
//
void listTransactionsByMonth(const Account& account) {
    std::string month = promptMonthKey("Month (YYYY-MM): ");
    auto results = account.getLedger().listByMonth(month);
    if (results.empty()) {
        std::cout << "No transactions for " << month << "\n";
        return;
    }
    for (const auto& tx : results) {
        // rely on the Transaction stream overload for output


        std::cout << tx << "\n";
    }
}

// Gives overview of specified monthly total (income/expense/net)
void showMonthlyTotals(const Account& account) {
    std::string month = promptMonthKey("Month (YYYY-MM): ");
    Money income(0);
    Money expense(0);
    for (const auto& tx : account.getLedger().listByMonth(month)) {
        if (tx.isIncome()) {
            // positive amounts add to the income tally
            income = income + tx.getAmount();
        } else {
            // expenses are stored negative so addition keeps the sign
            expense = expense + tx.getAmount();
        }
    }
    Money net = income + expense;
    Money expenseAbs(-expense.getMoney());
    std::cout << "Income: " << income.toString() << "\n";
    std::cout << "Expense: " << expenseAbs.toString() << "\n";
    std::cout << "Net: " << net.toString() << "\n";
}

// Searches transaction by keyword (one of the original methods from module 1)
void searchTransactionsByKeyword(const Account& account) {
    std::string keyword = readLine("Keyword: ");
    auto matches = account.getLedger().search(keyword);
    if (matches.empty()) {
        std::cout << "No transactions found.\n";
        return;
    }
    for (const auto& tx : matches) {
        // each match prints on its own line
        std::cout << tx << "\n";
    }
}

// Original code from module 1, reapplied in its own method
// Lets user filter transactions
void runTransactionFilters(const Account& account) {
    std::cout << "\nChoose filter:\n";
    std::cout << "1. List transactions by month\n";
    std::cout << "2. Show monthly totals (Income / Expense / Net)\n";
    std::cout << "3. Search transactions by keyword\n";
    int choice = promptIntInRange("Choice: ", 1, 3);
    if (choice == 1) {
        listTransactionsByMonth(account);
    } else if (choice == 2) {
        showMonthlyTotals(account);
    } else if (choice == 3) {
        searchTransactionsByKeyword(account);
    }
}

// Adds a normal new transaction (code from module 1)
void addNormalTransaction(AccountRecord& record, const CsvHandler& csvHandler) {
    std::cout << "\n--- New transaction ---\n";
    std::string id = promptNonEmpty("ID: ");
    std::string date = promptFullDate("Date (YYYY-MM-DD): ");
    Money amount = promptMoney("Amount ($12.34 or 12.34): ");
    Category category = promptCategory("Category (Food, Rent, Utilities, Transport, Income, Transfer, Other): ");
    std::string note = promptNonEmpty("Note: ");

    if (category == Category::Income && amount.getMoney() < 0) {
        amount.setMoney(-amount.getMoney());
    }
    if (category != Category::Income && category != Category::Transfer && amount.getMoney() > 0) {
        amount.setMoney(-amount.getMoney());
    }

    Transaction tx(id, date, amount, category, note);
    addTransactionAndPersist(record, tx, csvHandler);
    std::cout << "Transaction recorded.\n";
}

// Adds a recurring transaction based on a specified occurance
void addRecurringTransaction(AccountRecord& record, const CsvHandler& csvHandler) {
    std::cout << "\n--- New recurring transaction ---\n";
    std::string id = promptNonEmpty("Base ID: ");
    std::string startDate = promptFullDate("First occurrence (YYYY-MM-DD): ");
    Money amount = promptMoney("Amount per occurrence: ");
    Category category = promptCategory("Category (Food, Rent, Utilities, Transport, Income, Transfer, Other): ");
    std::string note = promptNonEmpty("Note: ");
    std::string frequency = promptFrequency();
    std::string targetMonth = promptMonthKey("Generate month (YYYY-MM): ");

    if (category == Category::Income && amount.getMoney() < 0) {
        amount.setMoney(-amount.getMoney());
    }
    if (category != Category::Income && category != Category::Transfer && amount.getMoney() > 0) {
        amount.setMoney(-amount.getMoney());
    }

    RecurringTransaction recurring(id, startDate, amount, category, frequency, note);
    auto generated = recurring.generateInstances(targetMonth);
    if (generated.empty()) {
        std::cout << "No recurring instances generated for the selected month.\n";
        return;
    }
    for (const auto& tx : generated) {
        addTransactionAndPersist(record, tx, csvHandler);
    }
    std::cout << generated.size() << " recurring transaction(s) added.\n";
}

// Intermediary code to handle transactions and let user specify which one
void handleTransactions(AccountRecord& record, const CsvHandler& csvHandler) {
    std::cout << "\n1. Normal transaction\n";
    std::cout << "2. Recurring transaction\n";
    std::cout << "0. Cancel\n";
    int choice = promptIntInRange("Choice: ", 0, 2);

    if (choice == 1) {
        addNormalTransaction(record, csvHandler);
    } else if (choice == 2) {
        addRecurringTransaction(record, csvHandler);
    }
    // choice 0 just drops back to the account menu
}

void handleTransfer(AccountRecord& current, std::vector<AccountRecord>& accounts, const CsvHandler& csvHandler) {
    std::cout << "\nSelect destination account:\n";
    std::vector<size_t> indices;
    size_t menuIndex = 1;
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].account.get() == current.account.get()) {
            continue;
        }
        std::cout << menuIndex << ": " << accounts[i].account->getName() << "\n";
        indices.push_back(i);
        ++menuIndex;
    }

    if (indices.empty()) {
        std::cout << "No other accounts are currently loaded. Load another account from CSV first.\n";
        return;
    }

    int choice = promptIntInRange("Choice: ", 1, static_cast<int>(indices.size()));
    AccountRecord& destination = accounts[indices[static_cast<size_t>(choice - 1)]];

    std::string date = promptFullDate("Date (YYYY-MM-DD): ");
    Money amount = promptMoney("Amount: ");
    if (amount.getMoney() < 0) {
        amount.setMoney(-amount.getMoney());
    }

    Money outgoing = amount;
    // outgoing should be negative to reflect funds leaving the source account
    outgoing.setMoney(-outgoing.getMoney());

    std::string baseId = "TR-" + current.account->getName() + "->" + destination.account->getName();
    Transaction fromTx(baseId + "-OUT", date, outgoing, Category::Transfer,
                       "Transfer to " + destination.account->getName());
    Transaction toTx(baseId + "-IN", date, amount, Category::Transfer,
                     "Transfer from " + current.account->getName());

    addTransactionAndPersist(current, fromTx, csvHandler);
    addTransactionAndPersist(destination, toTx, csvHandler);
    std::cout << "Transfer completed.\n";
}

void handleBudget(AccountRecord& record) {
    bool open = true;
    while (open) {
        std::cout << "\n--- Budget tools ---\n";
        std::cout << "1. Output budget variance\n";
        std::cout << "2. Set budget limit\n";
        std::cout << "0. Back\n";
        int choice = promptIntInRange("Choice: ", 0, 2);

        if (choice == 0) {
            open = false;
        } else if (choice == 1) {
            Category cat = promptCategory("Category (Food, Rent, Utilities, Transport, Income, Transfer, Other): ");
            std::string month = promptMonthKey("Month (YYYY-MM): ");
            Money total = record.account->getLedger().sumByCategory(month, cat);
            Money spent = absoluteMoney(total);
            Money variance = record.budget.variance(cat, spent);
            std::cout << "Limit: " << record.budget.getLimit(cat).toString() << "\n";
            std::cout << "Spent: " << spent.toString() << "\n";
            std::cout << "Variance: " << variance.toString() << "\n";
        } else if (choice == 2) {
            Category cat = promptCategory("Category (Food, Rent, Utilities, Transport, Income, Transfer, Other): ");
            Money limit = promptMoney("Limit amount: ");
            if (limit.getMoney() < 0) {
                limit.setMoney(-limit.getMoney());
            }
            record.budget.setLimit(cat, limit);
        }
    }
}


// Saves accounts on ledger to the CSV file
void saveAccountToCsv(AccountRecord& record, const CsvHandler& csvHandler) {
    std::string filename = record.csvFile;
    if (filename.empty()) {
        filename = readLine("CSV filename to write: ");
        record.csvFile = filename;
    } else {
        std::string answer = lowerCopy(readLine("Write to existing file '" + filename + "'? (y/n): "));
        if (answer == "n" || answer == "no") {
            // let the user redirect output to a different file
            filename = readLine("CSV filename to write: ");
            record.csvFile = filename;
        }
    }

    if (filename.empty()) {
        std::cout << "No filename provided.\n";
        return;
    }

    std::string effectiveType = accountTypeForRecord(record);
    record.accountType = effectiveType;
    csvHandler.writeLedger(filename,
                           record.account->getLedger(),
                           record.account->getName(),
                           effectiveType);
    std::cout << "Account saved to " << filename << "\n";
}

// Original account menu with print statements
void showAccountMenu() {
    std::cout << "\n===== Account Menu =====\n";
    std::cout << "1. Account overview\n";
    std::cout << "2. Find transfer with filter\n";
    std::cout << "3. Search transfers\n";
    std::cout << "4. Make new transaction\n";
    std::cout << "5. Make transfer\n";
    std::cout << "6. Budget\n";
    std::cout << "7. Save account to CSV\n";
    std::cout << "0. Back to main\n";
}

void handleAccountSession(AccountRecord& record, std::vector<AccountRecord>& accounts, const CsvHandler& csvHandler) {
    bool firstPass = true;
    bool inSession = true;
    while (inSession) {
        if (firstPass) {
            showAccountOverview(*record.account);
            firstPass = false;
        }
        showAccountMenu();
        int choice = promptIntInRange("Choice: ", 0, 7);

        switch (choice) {
            case 1:
                showAccountOverview(*record.account);
                break;
            case 2:
                runTransactionFilters(*record.account);
                break;
            case 3:
                searchTransfers(*record.account);
                break;
            case 4:
                handleTransactions(record, csvHandler);
                break;
            case 5:
                handleTransfer(record, accounts, csvHandler);
                break;
            case 6:
                handleBudget(record);
                break;
            case 7:
                saveAccountToCsv(record, csvHandler);
                break;
            case 0:
                inSession = false;
                break;
            default:
                break;
        }
    }
}

std::unique_ptr<Account> createAccountByType(const std::string& name, const std::string& type) {
    std::string lowered = lowerCopy(type);
    if (lowered == "credit") {
        return std::make_unique<CreditAccount>(name);
    }
    if (lowered == "current" || lowered == "checking") {
        return std::make_unique<CheckingAccount>(name);
    }
    if (lowered == "savings") {
        return std::make_unique<SavingsAccount>(name);
    }
    return nullptr;
}

AccountRecord* findAccountRecord(std::vector<AccountRecord>& accounts, const std::string& name) {
    std::string needle = lowerCopy(name);
    for (auto& record : accounts) {
        if (lowerCopy(record.account->getName()) == needle) {
            return &record;
        }
    }
    return nullptr;
}

const AccountRecord* findAccountRecord(const std::vector<AccountRecord>& accounts, const std::string& name) {
    std::string needle = lowerCopy(name);
    for (const auto& record : accounts) {
        if (lowerCopy(record.account->getName()) == needle) {
            return &record;
        }
    }
    return nullptr;
}

void loadTransactionsFromCsv(AccountRecord& record, const std::string& filename, const CsvHandler& csvHandler) {
    std::string detectedType = csvHandler.detectAccountType(filename);
    if (!detectedType.empty()) {
        std::string normalized = lowerCopy(detectedType);
        if (record.accountType.empty()) {
            record.accountType = normalized;
        } else if (record.accountType != normalized) {
            std::cout << "Warning: CSV indicates account type '" << normalized
                      << "' but current account is '" << record.accountType << "'.\n";
        }
    } else if (record.accountType.empty()) {
        record.accountType = accountTypeForRecord(record);
    }

    Ledger temp;
    csvHandler.readCSV(filename, temp);
    // wipe current ledger before layering in the file's entries
    record.account->clearTransactions();
    for (const auto& tx : temp.getAll()) {
        // reuse Account::addTransaction to keep ordering consistent
        record.account->addTransaction(tx);
    }
    record.csvFile = filename;
    std::cout << "Loaded " << temp.getAll().size() << " transaction(s) from " << filename << "\n";
}

// Entry point -------------------

int main() {
    std::vector<AccountRecord> accounts;
    CsvHandler csvHandler;

    bool running = true;
    while (running) {
        std::cout << "\n===== FinTrack Pro =====\n";
        std::cout << "1. Make new account\n";
        std::cout << "2. Load account from CSV\n";
        std::cout << "0. Exit\n";
        int choice = promptIntInRange("Choice: ", 0, 2);

        if (choice == 0) {
            running = false;
        } else if (choice == 1) {
            std::string name = promptNonEmpty("Account name: ");
            AccountRecord* existing = findAccountRecord(accounts, name);
            if (existing) {
                // if the account is already in memory just jump into its menu
                std::cout << "Account already exists. Opening session.\n";
                handleAccountSession(*existing, accounts, csvHandler);
                continue;
            }

            std::string type = promptAccountType();
            auto accountPtr = createAccountByType(name, type);
            if (!accountPtr) {
                std::cout << "Unknown account type.\n";
                continue;
            }

            std::string filename = readLine("CSV filename (leave blank for automatic): ");
            if (filename.empty()) {
                // default storage name mirrors account title
                filename = name + ".csv";
            }

            AccountRecord record;
            record.account = std::move(accountPtr);
            record.accountType = type;
            record.csvFile = filename;
            csvHandler.ensureHeader(filename, record.accountType);

            accounts.push_back(std::move(record));
            std::cout << "Account created.\n";
            handleAccountSession(accounts.back(), accounts, csvHandler);
        } else if (choice == 2) {
            std::string name = promptNonEmpty("Account name: ");
            std::string filename = promptNonEmpty("CSV filename: ");

            AccountRecord* existing = findAccountRecord(accounts, name);
            if (existing) {
                loadTransactionsFromCsv(*existing, filename, csvHandler);
                std::cout << "Account updated from CSV.\n";
                handleAccountSession(*existing, accounts, csvHandler);
                continue;
            }

            std::string detectedType = csvHandler.detectAccountType(filename);
            if (!detectedType.empty()) {
                std::cout << "Detected account type: " << detectedType << "\n";
            }
            std::string type = lowerCopy(detectedType);
            if (type == "checking") {
                type = "current";
            }
            if (type != "credit" && type != "current" && type != "savings") {
                // prompt user when the file doesn't carry type metadata
                type = promptAccountType();
            }

            auto accountPtr = createAccountByType(name, type);
            if (!accountPtr) {
                std::cout << "Unknown account type.\n";
                continue;
            }

            AccountRecord record;
            record.account = std::move(accountPtr);
            record.accountType = lowerCopy(type);
            loadTransactionsFromCsv(record, filename, csvHandler);

            accounts.push_back(std::move(record));
            handleAccountSession(accounts.back(), accounts, csvHandler);
        }
    }

    std::cout << "Goodbye!\n";
    return 0;
}
