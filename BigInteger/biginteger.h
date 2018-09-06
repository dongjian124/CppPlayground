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


    BigInteger operator+();
    BigInteger operator-();

    BigInteger&operator++();
    BigInteger operator++(int);
    BigInteger&operator--();
    BigInteger operator--(int);

    BigInteger operator+(const BigInteger&) const;
    BigInteger operator-(const BigInteger&) const;
    BigInteger operator*(const BigInteger&) const;
    BigInteger operator/(const BigInteger&) const;
    BigInteger operator%(const BigInteger&) const;

    BigInteger&operator+=(const BigInteger&);
    BigInteger&operator-=(const BigInteger&);
    BigInteger&operator*=(const BigInteger&);
    BigInteger&operator/=(const BigInteger&);
    BigInteger&operator%=(const BigInteger&);

    bool operator == (const BigInteger&) const;
    bool operator != (const BigInteger&) const;
    bool operator < (const BigInteger&) const;
    bool operator <= (const BigInteger&) const;
    bool operator > (const BigInteger&) const;
    bool operator >= (const BigInteger&) const;


    BigInteger operator=(const std::string&);


    BigInteger(long long value = 0)
    {
        std::stringstream ss;
        ss << value;
        this->operator=(ss.str());
    }
    BigInteger(const std::string& s)
    {
        this->operator=(s);
    }

    bool IsNegetive();

private:

    std::vector<int32_t> number_;
    bool negative_;

    void trim_prefix_zeros();
};

#endif //PROJECT_BIGINTEGER_H
