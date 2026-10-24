// In-memory view of every account CSV in a data directory.
// All public methods are thread safe; every mutation is written back to disk immediately.

#ifndef FINTRACK_STORE_H
#define FINTRACK_STORE_H

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <json.hpp>

#include "accounts/Account.h"
#include "io/CsvHandler.h"
#include "users/Budget.h"

class Store {
private:
    struct StoredAccount {
        std::unique_ptr<Account> account;
        std::string type;
        std::filesystem::path file;
    };

    std::filesystem::path dataDir;
    std::vector<StoredAccount> accounts;
    Budget budget;
    CsvHandler csv;
    mutable std::mutex mutex;

    StoredAccount* find(const std::string& name);
    void save(const StoredAccount& stored) const;
    static std::unique_ptr<Account> makeAccount(const std::string& name, const std::string& type);
    static nlohmann::json accountJson(const StoredAccount& stored);

public:
    explicit Store(std::filesystem::path dir);

    // (re)read every *.csv in the data directory
    void load();

    nlohmann::json accountsJson() const;
    nlohmann::json transactionsJson() const;
    nlohmann::json budgetsJson() const;

    // mutations throw std::invalid_argument with a user facing message on bad input
    nlohmann::json addAccount(const std::string& name, const std::string& type);
    nlohmann::json addTransaction(const std::string& account, const std::string& date, long long cents,
                                  const std::string& category, const std::string& note);
    void transfer(const std::string& from, const std::string& to, const std::string& date, long long cents);
    void setBudget(const std::string& category, long long cents);
};

#endif //FINTRACK_STORE_H
