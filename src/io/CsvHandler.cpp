//
// Created by Tim Mäser on 30/09/2025.
//

#include "CsvHandler.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "core/Ledger.h"
#include "core/money.h"
#include "core/category.h"
#include "core/transaction.h"


CsvHandler::CsvHandler() = default;

// helper that keeps type comparisons case-insensitive
static std::string normalizeType(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return type;
}

// make sure a file exists and has the header lines
void CsvHandler::ensureHeader(const std::string& filename, const std::string& accountType) const {
    std::ifstream input(filename);
    if (input.good()) {
        int ch = input.peek();
        if (ch != std::ifstream::traits_type::eof()) {
            input.close();
            return;
        }
    }
    input.close();

    std::ofstream output(filename, std::ios::out | std::ios::trunc);
    if (!output.is_open()) {
        return;
    }
    std::string typeLine = normalizeType(accountType);
    if (!typeLine.empty()) {
        output << "#AccountType=" << typeLine << std::endl;
    }
    output << "ID,Date,Account,Category,Amount,Note" << std::endl;
    output.close();
}

// append a single transaction row to the CSV
void CsvHandler::appendTransaction(const std::string& filename,
                                   const Transaction& tx,
                                   const std::string& accountName,
                                   const std::string& accountType) const {
    ensureHeader(filename, accountType);
    std::ofstream output(filename, std::ios::app);
    if (!output.is_open()) {
        return;
    }
    output << tx.getID() << ","
           << tx.getDate() << ","
           << accountName << ","
           << categoryToString(tx.getCategory()) << ","
           << tx.getAmount().toString() << ","
           << tx.getNote()
           << std::endl;
    output.close();
}

// rewrite the full ledger to disk (used for manual saves)
void CsvHandler::writeLedger(const std::string& filename,
                             const Ledger& ledger,
                             const std::string& accountName,
                             const std::string& accountType) const {
    std::ofstream output(filename, std::ios::out | std::ios::trunc);
    if (!output.is_open()) {
        return;
    }
    std::string typeLine = normalizeType(accountType);
    if (!typeLine.empty()) {
        output << "#AccountType=" << typeLine << std::endl;
    }
    output << "ID,Date,Account,Category,Amount,Note" << std::endl;
    for (const auto& tx : ledger.getAll()) {
        output << tx.getID() << ","
               << tx.getDate() << ","
               << accountName << ","
               << categoryToString(tx.getCategory()) << ","
               << tx.getAmount().toString() << ","
               << tx.getNote()
               << std::endl;
    }
    output.close();
}

// try to read the stored account type comment from disk
std::string CsvHandler::detectAccountType(const std::string& filename) const {
    std::ifstream inputFile(filename, std::ios::in);
    if (!inputFile.is_open()) {
        return {};
    }
    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }
        if (line[0] == '#') {
            std::string key = "#AccountType=";
            if (line.rfind(key, 0) == 0 && line.size() > key.size()) {
                std::string value = line.substr(key.size());
                inputFile.close();
                return normalizeType(value);
            }
            continue;
        }
        break;
    }
    inputFile.close();
    return {};
}

// load transactions from CSV while skipping metadata lines
void CsvHandler::readCSV(const std::string& filename, Ledger& ledger) const {
    std::ifstream inputFile(filename, std::ios::in);
    if (inputFile.is_open()) {
        std::string line;
        bool headerSkipped = false;
        while (std::getline(inputFile, line)) {
            if (line.empty()) {
                continue;
            }
            if (line[0] == '#') {
                continue;
            }
            if (!headerSkipped) {
                headerSkipped = true;
                continue;
            }
            std::stringstream ss(line);
            std::string id, date, account, category, amount, note;

            if (!std::getline(ss, id, ',')) continue;
            if (!std::getline(ss, date, ',')) continue;
            if (!std::getline(ss, account, ',')) continue;
            if (!std::getline(ss, category, ',')) continue;
            if (!std::getline(ss, amount, ',')) continue;
            if (!std::getline(ss, note, ',')) note = "";
            Money money = Money::fromString(amount);
            Category cat = stringToCategory(category);
            Transaction tx(id, date, money, cat, note);

            ledger.add(tx);



        }

        inputFile.close();
    }
}
