//
// Created by sb on 18-9-5.
//

#include "biginteger.h"
#include <algorithm>

inline bool ok(int next_value)
{
    if (!isdigit(next_value))
        exit(1);
    return true;
}

std::istream &operator>>(std::istream &in, BigInteger &rhs)
{
    std::string s;
    if (!(in >> s))
        return in;
    rhs = s;
    return in;
}

std::ostream &operator<<(std::ostream &out, const BigInteger &rhs)
{

}

BigInteger BigInteger::operator+(const BigInteger & rhs)
{

}

BigInteger BigInteger::operator-(const BigInteger & rhs)
{

}

BigInteger BigInteger::operator*(const BigInteger & rhs)
{

}

BigInteger BigInteger::operator/(const BigInteger &) {}
BigInteger BigInteger::operator%(const BigInteger &) {}

BigInteger &BigInteger::operator+=(const BigInteger &rhs) {}
BigInteger &BigInteger::operator-=(const BigInteger &rhs) {}
BigInteger &BigInteger::operator*=(const BigInteger &rhs) {}
BigInteger &BigInteger::operator/=(const BigInteger &rhs) {}
BigInteger &BigInteger::operator%=(const BigInteger &rhs) {}

bool BigInteger::operator==(const BigInteger &rhs) {}
bool BigInteger::operator!=(const BigInteger &rhs) { return !(*this == rhs); }
bool BigInteger::operator<(const BigInteger &rhs) {}
bool BigInteger::operator<=(const BigInteger &rhs) {}
bool BigInteger::operator>(const BigInteger &rhs) { return !(*this <= rhs); }
bool BigInteger::operator>=(const BigInteger &rhs) { return !(*this < rhs); }

BigInteger BigInteger::operator=(const std::string & s)
{
    number_.clear();
    int idx = 0;
    while(isspace(s[idx]))
        ++idx;
    if(s[idx] == '-' || s[idx] == '+')
    {
        negative_ = s[idx] == '-';
        ++idx;
    }
    while(isspace(s[idx]))
        ++idx;
    while(s[idx] == '0')
        ++idx;
    while(ok(idx))
    {
        number_.push_back(s[idx] - '0');
        ++idx;
    }
    std::reverse(number_.begin(), number_.end());
    return *this;
}

BigInteger &BigInteger::operator++()
{
    if(!negative_)
    {
        ++number_.front();
        for(auto i = 0 ; i < number_.size() - 1; ++i)
        {
            if(number_[i] >= 10)
            {
                ++number_[i + 1];
                number_[i] -= 10;
            }
        }
    }
    else
    {
        if(number_.size() == 1 && number_[0] == 1)
            negative_ = false , number_[0] = 0;
        else
        {
            negative_ = false;
            --*this;
            negative_ = true;
        }
    }
    return *this;
}

BigInteger BigInteger::operator++(int)
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

BigInteger &BigInteger::operator--()
{
    if(negative_)
    {
        ++number_.front();
        for(auto i = 0; i < number_.size() - 1; ++i)
        {
            if(number_[i] >= 10)
            {
                ++number_[i + 1];
                number_[i] -= 10;
            }
        }
    }
    else
    {
        if(number_.size() == 1 && number_[0] == 1)
            negative_ = true , number_[0] = 0;
        else
        {
            --number_.front();
            for(auto i = 0 ; i < number_.size() - 1; ++i)
            {
                if(number_[i] < 0)
                {
                    --number_[i + 1];
                    number_[i] += 10;
                }
            }
        }
    }
    return *this;
}

BigInteger BigInteger::operator--(int)
{
    auto tmp = *this;
    --*this;
    return tmp;
}


