//
// Created by sb on 18-9-5.
//

#ifndef PROJECT_BIGINTEGER_H
#define PROJECT_BIGINTEGER_H

#include <string>
#include <vector>
#include <sstream>

class BigInteger
{
public:

    friend std::istream&operator>>(std::istream& , BigInteger&);
    friend std::ostream&operator<<(std::ostream&, const BigInteger&);


    BigInteger&operator++();
    BigInteger operator++(int);
    BigInteger&operator--();
    BigInteger operator--(int);

    BigInteger operator+(const BigInteger&);
    BigInteger operator-(const BigInteger&);
    BigInteger operator*(const BigInteger&);
    BigInteger operator/(const BigInteger&);
    BigInteger operator%(const BigInteger&);

    BigInteger&operator+=(const BigInteger&);
    BigInteger&operator-=(const BigInteger&);
    BigInteger&operator*=(const BigInteger&);
    BigInteger&operator/=(const BigInteger&);
    BigInteger&operator%=(const BigInteger&);

    bool operator == (const BigInteger&);
    bool operator != (const BigInteger&);
    bool operator < (const BigInteger&);
    bool operator <= (const BigInteger&);
    bool operator > (const BigInteger&);
    bool operator >= (const BigInteger&);


    BigInteger operator=(const std::string&);


    BigInteger(long long value)
    {
        this->operator=(std::to_string(value));
    }
    BigInteger(const std::string& s)
    {
        this->operator=(s);
    }

    bool IsNegetive();

private:

    std::vector<int32_t> number_;
    bool negative_;

};

#endif //PROJECT_BIGINTEGER_H
