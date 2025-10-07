//
// Created by Tim Mäser on 30/09/2025.
//

#ifndef M2OEP_TMASER_CSVHANDLER_H
#define M2OEP_TMASER_CSVHANDLER_H
#include <string>
#include "core/Ledger.h"


class CsvHandler {
public:
    CsvHandler();

    // ensure the CSV exists and starts with our header/meta info
    void ensureHeader(const std::string& filename, const std::string& accountType) const;
    // append a single transaction row to the CSV file
    void appendTransaction(const std::string& filename,
                           const Transaction& tx,
                           const std::string& accountName,
                           const std::string& accountType) const;
    // dump an entire ledger to the specified CSV
    void writeLedger(const std::string& filename,
                     const Ledger& ledger,
                     const std::string& accountName,
                     const std::string& accountType) const;
    // read any stored account type metadata from disk
    std::string detectAccountType(const std::string& filename) const;
    // populate a ledger from the CSV contents
    void readCSV(const std::string& filename, Ledger& ledger) const;
};


#endif //M2OEP_TMASER_CSVHANDLER_H
