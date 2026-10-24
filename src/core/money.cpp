// Money implementation
// math happens in integer space, rounding only when parsing strings
#include "core/money.h"
#include <cmath>
#include <string>


    //Basic constructor setting it to 0 or input val.
    Money::Money() : cents(0) {}
    Money::Money(long long c) : cents(c) {}

    bool Money:: operator<(const Money &other) const {
        return cents < other.cents;
    }
    bool Money:: operator>(const Money &other) const {
        return cents > other.cents;
    }
    bool Money:: operator<=(const Money &other) const {
        return cents <= other.cents;
    }
    bool Money:: operator>=(const Money &other) const {
        return cents >= other.cents;
    }
    bool Money:: operator==(const Money &other) const {
        return cents == other.cents;
    }

    long long Money::getMoney() const {
        return cents;
    }
    void Money::setMoney(long long money) {
        cents = money;
    }
    Money Money::operator+(const Money& other) const {
        return {this->cents + other.cents};
    }
    Money Money::operator-(const Money& other) const {
        return {this->cents - other.cents};
    }
    Money Money::operator*(double factor) const {
        long long result = static_cast<long long>(std::round(cents * factor));
        return Money(result);
    }

    // fromString method allows users to include a dollar sign or not,
    // to make input easier for users.
    Money Money::fromString(const std::string &str) {
            std::string s = str;

            // accept a leading sign before the dollar sign (eg -$12.34)
            bool negative = false;
            if (!s.empty() && (s[0] == '-' || s[0] == '+')) {
                negative = s[0] == '-';
                s = s.substr(1);
            }

            // Remove $ if present
            if (!s.empty() && s[0] == '$') {
                s = s.substr(1);
            }
            if (negative) {
                s = "-" + s;
            }

            // convert to double (eg 12.34 -> 12.34)
            double value = std::stod(s);

            // convert dollars to cents, round to nearest
            long long cents = static_cast<long long>(std::round(value * 100));

            return {cents};
    }

    // easy visible output for user.
    std::string Money::toString() const {
        // work on the magnitude so values like -0.50 keep their sign
        long long magnitude = cents < 0 ? -cents : cents;
        long long dollars = magnitude / 100;
        long long remainder = magnitude % 100;
        // format as $D.CC or -$D.CC; keep it compact
        return std::string(cents < 0 ? "-$" : "$") + std::to_string(dollars) + "." + (remainder < 10 ? "0" : "") + std::to_string(remainder);
    }
