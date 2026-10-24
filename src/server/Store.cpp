// Store implementation
// Wraps the core account classes so the HTTP layer only deals with JSON.

#include "Store.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>

#include "accounts/CheckingAccount.h"
#include "accounts/CreditAccount.h"
#include "accounts/SavingsAccount.h"
#include "core/category.h"
#include "transactions/TransferTransaction.h"
#include "utility.h"

namespace fs = std::filesystem;
using nlohmann::json;

static const char* kBudgetFile = "budgets.csv";

// ids only need to be unique within the data directory
static std::string newId() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    static long long last = 0;
    long long id = std::max(last + 1, static_cast<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count()));
    last = id;
    return "T" + std::to_string(id);
}

Store::Store(fs::path dir) : dataDir(std::move(dir)) {}

std::unique_ptr<Account> Store::makeAccount(const std::string& name, const std::string& type) {
    if (type == "savings") return std::make_unique<SavingsAccount>(name);
    if (type == "credit") return std::make_unique<CreditAccount>(name);
    return std::make_unique<CheckingAccount>(name);
}

void Store::load() {
    std::lock_guard<std::mutex> lock(mutex);
    accounts.clear();
    budget = Budget();
    fs::create_directories(dataDir);

    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(dataDir)) {
        if (entry.path().extension() == ".csv" && entry.path().filename() != kBudgetFile) {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());

    for (const auto& file : files) {
        std::string type = csv.detectAccountType(file.string());
        if (type.empty()) type = "checking";
        StoredAccount stored{makeAccount(file.stem().string(), type), type, file};
        Ledger ledger;
        csv.readCSV(file.string(), ledger);
        for (const auto& tx : ledger.getAll()) {
            stored.account->addTransaction(tx);
        }
        accounts.push_back(std::move(stored));
    }
    csv.readBudget((dataDir / kBudgetFile).string(), budget);
}

Store::StoredAccount* Store::find(const std::string& name) {
    for (auto& stored : accounts) {
        if (stored.account->getName() == name) return &stored;
    }
    return nullptr;
}

void Store::save(const StoredAccount& stored) const {
    csv.writeLedger(stored.file.string(), stored.account->getLedger(), stored.account->getName(), stored.type);
}

json Store::accountJson(const StoredAccount& stored) {
    json out = {
        {"name", stored.account->getName()},
        {"type", stored.type},
        {"balance", stored.account->getLedger().total().getMoney()},
        {"transactionCount", stored.account->getLedger().getAll().size()},
    };
    if (auto* savings = dynamic_cast<const SavingsAccount*>(stored.account.get())) {
        out["interestRate"] = savings->getInterestRate();
    }
    return out;
}

json Store::accountsJson() const {
    std::lock_guard<std::mutex> lock(mutex);
    json out = json::array();
    for (const auto& stored : accounts) out.push_back(accountJson(stored));
    return out;
}

json Store::transactionsJson() const {
    std::lock_guard<std::mutex> lock(mutex);
    json out = json::array();
    for (const auto& stored : accounts) {
        for (const auto& tx : stored.account->getLedger().getAll()) {
            out.push_back({
                {"id", tx.getID()},
                {"date", tx.getDate()},
                {"account", stored.account->getName()},
                {"category", categoryToString(tx.getCategory())},
                {"amount", tx.getAmount().getMoney()},
                {"note", tx.getNote()},
            });
        }
    }
    return out;
}

json Store::budgetsJson() const {
    std::lock_guard<std::mutex> lock(mutex);
    json out = json::array();
    for (const auto& [cat, limit] : budget.getLimits()) {
        out.push_back({{"category", categoryToString(cat)}, {"limit", limit.getMoney()}});
    }
    return out;
}

json Store::addAccount(const std::string& name, const std::string& type) {
    std::lock_guard<std::mutex> lock(mutex);
    // the name doubles as the file name, so keep it to a safe character set
    bool safe = !name.empty() && name.size() <= 40 && std::all_of(name.begin(), name.end(), [](unsigned char c) {
        return std::isalnum(c) || c == ' ' || c == '-' || c == '_';
    });
    if (!safe) throw std::invalid_argument("Account names may only contain letters, numbers, spaces, - and _");
    if (type != "checking" && type != "savings" && type != "credit") {
        throw std::invalid_argument("Account type must be checking, savings or credit");
    }
    if (find(name)) throw std::invalid_argument("An account with that name already exists");

    StoredAccount stored{makeAccount(name, type), type, dataDir / (name + ".csv")};
    save(stored);
    accounts.push_back(std::move(stored));
    return accountJson(accounts.back());
}

json Store::addTransaction(const std::string& account, const std::string& date, long long cents,
                           const std::string& category, const std::string& note) {
    std::lock_guard<std::mutex> lock(mutex);
    StoredAccount* stored = find(account);
    if (!stored) throw std::invalid_argument("Unknown account: " + account);
    if (!validDateFull(date)) throw std::invalid_argument("Date must be YYYY-MM-DD");
    if (!validCat(category)) throw std::invalid_argument("Unknown category: " + category);
    if (note.find('\n') != std::string::npos) throw std::invalid_argument("Notes must be a single line");

    Transaction tx(newId(), date, Money(cents), stringToCategory(category), note);
    stored->account->addTransaction(tx);
    save(*stored);
    return {{"id", tx.getID()}, {"date", date}, {"account", account},
            {"category", categoryToString(tx.getCategory())}, {"amount", cents}, {"note", note}};
}

void Store::transfer(const std::string& from, const std::string& to, const std::string& date, long long cents) {
    std::lock_guard<std::mutex> lock(mutex);
    StoredAccount* source = find(from);
    StoredAccount* destination = find(to);
    if (!source || !destination) throw std::invalid_argument("Unknown account");
    if (source == destination) throw std::invalid_argument("Choose two different accounts");
    if (cents <= 0) throw std::invalid_argument("Transfer amount must be positive");
    if (!validDateFull(date)) throw std::invalid_argument("Date must be YYYY-MM-DD");

    TransferTransaction transfer(newId(), date, Money(cents), Category::Transfer, "");
    if (!transfer.execute(*source->account, *destination->account)) {
        throw std::invalid_argument("Transfers from a credit card into savings are not allowed");
    }
    save(*source);
    save(*destination);
}

void Store::setBudget(const std::string& category, long long cents) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!validCat(category)) throw std::invalid_argument("Unknown category: " + category);
    if (cents < 0) throw std::invalid_argument("Budget limit cannot be negative");
    budget.setLimit(stringToCategory(category), Money(cents));
    csv.writeBudget((dataDir / kBudgetFile).string(), budget);
}
