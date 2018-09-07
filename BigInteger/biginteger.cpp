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

BigInteger BigInteger::operator-()
{
    auto copy(*this);
    if(number_.size() != 1 || number_.front())
        copy.negative_ = !negative_;
    return copy;
}

BigInteger BigInteger::operator+()
{
    return *this;
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
    for (auto it = rhs.number_.rbegin(); it != rhs.number_.rend(); ++it)
        out << *it;
    return out;
}

BigInteger BigInteger::operator+(const BigInteger &rhs) const
{
    auto copy(*this);
    if (negative_ == rhs.negative_)
    {
        auto n = copy.number_.size(), m = rhs.number_.size();
        if(n < m)
            copy.number_.resize(m);
        for(auto i = 0; i < m; ++i)
            copy.number_[i] += rhs.number_[i];
        for(auto i = 0; i < n - 1; ++i)
        {
            if(copy.number_[i] >= 10)
            {
                ++copy.number_[i + 1];
                copy.number_[i] -= 10;
            }
        }
    }
    else
        copy.negative_^= 1, copy -= rhs , copy.negative_ ^= 1;
    if(copy.number_.size() == 1 && !copy.number_.front())
        copy.negative_ = false;
    return copy;
}

BigInteger BigInteger::operator-(const BigInteger &rhs) const
{
    auto copy(*this);
    if(negative_ == rhs.negative_)
    {
        auto n = copy.number_.size() , m = rhs.number_.size();
        if(n < m)
            copy.number_.resize(m);
        if((copy.number_  > rhs.number_) ^ copy.negative_) // abs(lhs) > abs(rhs)
        {
            for(auto i = 0 ; i < m; ++i)
                copy.number_[i] -= rhs.number_[i];
            for(auto i = 0 ; i < n - 1; ++i)
            {
                if(copy.number_[i] < 0)
                {
                    --copy.number_[i + 1];
                    copy.number_[i] += 10;
                }
            }
        }
        else
        {
            copy.negative_ ^= 1;
            for(auto i = 0; i< m; ++i)
                copy.number_[i] = rhs.number_[i] - copy.number_[i];
            for(auto i = 0; i < n - 1; ++i)
            {
                if(copy.number_[i] < 0)
                {
                    --copy.number_[i + 1];
                    copy.number_[i] += 10;
                }
            }
        }
    }
    else
        copy.negative_ ^= 1, copy += rhs, copy.negative_ ^= 1;
    copy.trim_prefix_zeros();
    return copy;
}

void BigInteger::trim_prefix_zeros()
{
    auto n = number_.size() - 1;
    while(!number_[n] && n)
        --n;
    number_.resize(n + 1);
    if(!n && !number_.back())
        negative_ = false;
}

BigInteger BigInteger::operator*(const BigInteger &rhs) const
{
    auto n = number_.size() , m = rhs.number_.size();
    BigInteger mul;
    if((n == 1 && number_[0] == 0) || (m == 1 && rhs.number_[0] == 0))
        return mul;
    mul.number_.resize(n + m);
    for(auto i = 0; i < n; ++i)
    {
        auto carry = 0;
        for(auto j = 0 ; j < m; ++j)
        {
            carry += number_[i] * rhs.number_[j] + mul.number_[i + j];
            mul.number_[i + j] = carry % 10;
            carry /= 10;
        }
        mul.number_[i + m] = carry;
    }
    mul.negative_ = negative_ ^ rhs.negative_;
    mul.trim_prefix_zeros();
    return mul;
}

int BigInteger::Sub(std::vector<int>& lhs , const std::vector<int> &rhs, int len1, int len2)
{
    int i;
    if(len1 < len2)
        return -1;
    if(len1 == len2)
    {
        for(i = len1 - 1; i >= 0 ; --i)
        {
            if(lhs[i] < rhs[i])
                return -1;
            else if(lhs[i] > rhs[i])
                break;
        }
    }
    for(i = 0; i < len1; ++i)
    {
        lhs[i] -= rhs[i];
        if(lhs[i] < 0)
        {
            lhs[i] += 10;
            lhs[i + 1];
        }
    }
    for(i = len1 - 1; i >= 0; --i)
        if(lhs[i])
            return i + 1;
    return 0;
}

BigInteger BigInteger::operator/(BigInteger  rhs)
{
    BigInteger ret , lhs(*this);

    int len1 = lhs.number_.size() , len2 = rhs.number_.size();
    if(len1 < len2)
        return ret;
    rhs.number_.resize(len1);
    len1 = Sub(lhs.number_, rhs.number_, len1, len2);
    if(len1 < 0)
        return ret;
    else if(len1 == 0)
        return BigInteger(1);
    ret.number_.resize(len1);
    ++ret.number_[0];
    int times = len1 - len2;
    if(times < 0)
        goto OutPutResult;
    else if(times > 0)
    {
        for(auto i = len1 - 1; i >= 0; --i)
        {
            if(i >= times)
                rhs[i] = rhs[i - times];
            else
                rhs[i] = 0;
        }
    }
    len2 = len1;
    for(auto j = 0; j <= times; ++j)
    {
        int tmp;
        while((tmp = Sub(lhs.number_ , rhs.number_.begin() + j , len1 , len2 - j)) >= 0)
        {
            len1 = tmp;
            ++ret.number_[times - j];
        }
    }
OutPutResult:
    for(auto i = 0 ; i < ret.number_.size(); ++i)
    {
        if(ret.number_[i] >= 10)
        {
            ret.number_[i + 1] += ret.number_[i] / 10;
            ret.number_[i] %= 10;
        }
    }
    return ret;
}

BigInteger BigInteger::operator%(const BigInteger &) const
{

}

BigInteger &BigInteger::operator+=(const BigInteger &rhs)
{
    *this = *this + rhs;
    return *this;
}

BigInteger &BigInteger::operator-=(const BigInteger &rhs)
{
    *this = *this - rhs;
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &rhs)
{
    *this = *this * rhs;
    return *this;
}

BigInteger &BigInteger::operator/=(const BigInteger &rhs)
{
    *this = *this / rhs;
    return *this;
}
BigInteger &BigInteger::operator%=(const BigInteger &rhs)
{
    *this = *this % rhs;
    return *this;
}

bool BigInteger::operator==(const BigInteger &rhs) const
{
   return negative_ == rhs.negative_ && number_ == rhs.number_;
}

bool BigInteger::operator!=(const BigInteger &rhs) const { return !(*this == rhs); }
bool BigInteger::operator<(const BigInteger &rhs) const
{
    if (!negative_ && !rhs.negative_)
    {
        int n = number_.size(), m = rhs.number_.size();
        if (n < m)
            return true;
        else if (n > m)
            return false;
        else
        {
            for (int i = n - 1; i >= 0; --i)
            {
                if (number_[i] > rhs.number_[i])
                    return false;
                else if (number_[i] < rhs.number_[i])
                    return true;
            }
            return false;
        }
    } else if (!negative_ && rhs.negative_)
        return false;
    else if (negative_ && !rhs.negative_)
        return true;
    else
    {
        int n = number_.size(), m = rhs.number_.size();
        if (n < m)
            return false;
        else if (n > m)
            return true;
        else
        {
            for (int i = n - 1; i >= 0; --i)
            {
                if (number_[i] > rhs.number_[i])
                    return true;
                else if (number_[i] < rhs.number_[i])
                    return false;
            }
            return false;
        }
    }
}

bool BigInteger::operator<=(const BigInteger &rhs) const
{
    if (!negative_ && !rhs.negative_)
    {
        int n = number_.size(), m = rhs.number_.size();
        if (n < m)
            return true;
        else if (n > m)
            return false;
        else
        {
            for (int i = n - 1; i >= 0; --i)
            {
                if (number_[i] > rhs.number_[i])
                    return false;
                else if (number_[i] < rhs.number_[i])
                    return true;
            }
            return true;
        }
    } else if (!negative_ && rhs.negative_)
        return false;
    else if (negative_ && !rhs.negative_)
        return true;
    else
    {
        int n = number_.size(), m = rhs.number_.size();
        if (n < m)
            return false;
        else if (n > m)
            return true;
        else
        {
            for (int i = n - 1; i >= 0; --i)
            {
                if (number_[i] > rhs.number_[i])
                    return true;
                else if (number_[i] < rhs.number_[i])
                    return false;
            }
            return true;
        }
    }
}

bool BigInteger::operator>(const BigInteger &rhs) const { return !(*this <= rhs); }
bool BigInteger::operator>=(const BigInteger &rhs) const { return !(*this < rhs); }

BigInteger BigInteger::operator=(const std::string &s)
{
    number_.clear();
    int idx = 0;
    while (idx < s.size() && isspace(s[idx]))
        ++idx;
    if (s[idx] == '-' || s[idx] == '+')
    {
        negative_ = s[idx] == '-';
        ++idx;
    }
    else
        negative_ = true;
    while (idx < s.size() && isspace(s[idx]))
        ++idx;
    while (idx < s.size() && ok(s[idx]))
    {
        number_.push_back(s[idx] - '0');
        ++idx;
    }
    std::reverse(number_.begin(), number_.end());
    return *this;
}

BigInteger &BigInteger::operator++()
{
    if (!negative_)
    {
        ++number_.front();
        for (auto i = 0; i < number_.size() - 1; ++i)
        {
            if (number_[i] >= 10)
            {
                ++number_[i + 1];
                number_[i] -= 10;
            }
            else
                break;
        }
    } else
    {
        if (number_.size() == 1 && number_[0] == 1)
            negative_ = false, number_[0] = 0;
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
    if (negative_)
    {
        ++number_.front();
        for (auto i = 0; i < number_.size() - 1; ++i)
        {
            if (number_[i] >= 10)
            {
                ++number_[i + 1];
                number_[i] -= 10;
            }
            else
                break;
        }
    } else
    {
        if (number_.size() == 1 && number_[0] == 1)
            negative_ = false, number_[0] = 0;
        else
        {
            --number_.front();
            for (auto i = 0; i < number_.size() - 1; ++i)
            {
                if (number_[i] < 0)
                {
                    --number_[i + 1];
                    number_[i] += 10;
                }
                else
                    break;
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


