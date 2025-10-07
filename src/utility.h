#ifndef UTILITY_H
#define UTILITY_H

#include <string>

// validation helpers used by the UI
bool validDateFull(const std::string& date);
// expect YYYY-MM
bool validDateMonth(const std::string& month);
// dollars and cents like 12.34 or $12.34
bool validAmount(const std::string& s);
// category name check, case insensitive
bool validCat(const std::string& s);

#endif
