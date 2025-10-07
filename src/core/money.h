// Money type for FinTrack Pro
// Stores cents as a 64 bit integer to avoid float drift.

#ifndef MONEY_H
#define MONEY_H

#ifndef M1OEP_TMASER_MONEY_H
#define M1OEP_TMASER_MONEY_H

#include <string>


class Money {
private:
    long long cents;
public:
    // default to zero cents
    Money();
    // construct directly from cents
    Money(long long c);

    // comparisons compare raw cents (helper methods essentially)
    bool operator<(const Money& m) const;
    bool operator>(const Money& m) const;
    bool operator<=(const Money& m) const;
    bool operator>=(const Money& m) const;
    bool operator==(const Money& m) const;

    // basic arithmetic, safe on cents
    Money operator+(const Money& money) const;
    Money operator-(const Money& money) const;
    Money operator*(double factor) const;

    // getter and setter for money
    void setMoney(long long money);
    long long getMoney() const;

    // produce a user facing string like $12.34
    std::string toString() const;



    // parse user input like 12.34 or $12.34 into cents
    // throws on invalid numeric input upstream, so callers should validate
    static Money fromString(const std::string & string);
};


#endif //M1OEP_TMASER_MONEY_H
#endif
