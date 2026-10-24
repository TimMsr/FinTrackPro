//
// Created by Tim Mäser on 01/10/2025.
//

#include "RecurringTransaction.h"

#include <algorithm>

// number of days in a YYYY-MM month, leap years included
static int daysInMonth(const std::string& month) {
    int year = std::stoi(month.substr(0, 4));
    int mon = std::stoi(month.substr(5, 2));
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    return (mon == 2 && leap) ? 29 : days[mon - 1];
}

// YYYY-MM plus a day number -> YYYY-MM-DD
static std::string dateFor(const std::string& month, int day) {
    return month + (day < 10 ? "-0" : "-") + std::to_string(day);
}


// Constructor forwards parameters to the base Transaction constructor
RecurringTransaction::RecurringTransaction(const std::string& id,
                         const std::string& date,
                         const Money& amount,
                         Category category,
                         const std::string& note)
    : Transaction(id, date, amount, category, note) {}



// build a batch of transactions based on the stored cadence
std::vector<Transaction> RecurringTransaction::generateInstances(const std::string &month) const {
    std::vector<Transaction> instances;
    int lastDay = daysInMonth(month);
    // keep the day of month from the start date, clamped for short months
    int anchorDay = std::min(std::stoi(getDate().substr(8, 2)), lastDay);

    if (frequency == "monthly") {
        instances.emplace_back(getID() + "-R", dateFor(month, anchorDay),
                               getAmount(), getCategory(), getNote() + " (Recurring)");
    } else if (frequency == "weekly") {
        // one entry every seven days, starting from the anchor day's position in the week
        for (int day = (anchorDay - 1) % 7 + 1; day <= lastDay; day += 7) {
            instances.emplace_back(getID() + "-R" + std::to_string(day), dateFor(month, day),
                                   getAmount(), getCategory(), getNote() + " (Recurring Weekly)");
        }
    } else if (frequency == "daily") {
        for (int day = 1; day <= lastDay; day++) {
            instances.emplace_back(getID() + "-R" + std::to_string(day), dateFor(month, day),
                                   getAmount(), getCategory(), getNote() + " (Recurring Daily)");
        }
    }
    return instances;
}

// update how often this template should repeat
void RecurringTransaction::setFrequency(const std::string &freq) {
    frequency = freq;
}
// expose the current frequency string
std::string RecurringTransaction::getFrequency() const {
    return frequency;
}
