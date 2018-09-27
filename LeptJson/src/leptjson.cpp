//
// Created by sb on 18-9-26.
//

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include "leptjson.h"

#define EXPECT(c, ch) do { assert(*c.json == (ch)); c.json++; } while(0)
#define ISDIGIT(ch)  ((ch >= '0') && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch >= '1') && (ch <= '9'))

class LeptContext
{
public:
    const char *json;
};

static void LeptParseWhiteSpace(LeptContext &c)
{
    auto p = c.json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c.json = p;
}

static LeptParseStatus LeptParseNumber(LeptContext &c, LeptJson &v)
{
    const char *p = c.json;
    if (*p == '-')
        p++;
    if (*p == '0')
        p++;
    else
    {
        if (!ISDIGIT1TO9(*p))
            return LeptParseStatus::kLeptParseInvalidValue;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.')
    {
        p++;
        for (; ISDIGIT(*p); p++);
    }
    if (*p == 'E' || *p == 'e')
        p++;
    if (*p == '-' || *p == '+')
        p++;

    while (ISDIGIT(*p))
        p++;
    if (*p != '\0')
        return LeptParseStatus::kLeptParseInvalidValue;
    v.n_ = strtod(c.json, nullptr);
    if (errno == ERANGE && (v.n_ == HUGE_VAL || v.n_ == -HUGE_VAL))
        return LeptParseStatus::kLeptParseInvalidValue;
    v.type_ = LeptType::kLeptNumber;
    c.json = p;
    return LeptParseStatus::kLeptParseOk;
}

static LeptParseStatus LeptParseLiterla(LeptContext &c, LeptJson &v, const char *literla, LeptType type)
{
    size_t  i;
    EXPECT(c, literla[0]);
    for (i = 0; literla[i + 1]; ++i)
    {
        if (c.json[i] != literla[i + 1])
            return LeptParseStatus::kLeptParseInvalidValue;
    }
    c.json += i;
    v.type_ = type;
    return LeptParseStatus::kLeptParseOk;
}

static LeptParseStatus LeptParseValue(LeptContext &c, LeptJson &v)
{
    switch (*c.json)
    {
        case 'n': return LeptParseLiterla(c, v, "null", LeptType::kLeptNull);
        case 't': return LeptParseLiterla(c, v, "true", LeptType::kLeptTrue);
        case 'f': return LeptParseLiterla(c, v, "false", LeptType::kLeptFalse);
        default: return LeptParseNumber(c, v);
        case '\0': return LeptParseStatus::kLeptParseExpectValue;
    }
}

LeptParseStatus LeptParse(LeptJson &v, const char *json)
{
    LeptContext c;
    c.json = json;
    v.type_ = LeptType::kLeptNull;
    LeptParseWhiteSpace(c);
    auto ret = LeptParseValue(c, v);
    if (ret == LeptParseStatus::kLeptParseOk)
    {
        LeptParseWhiteSpace(c);
        if (*c.json != '\0')
            return LeptParseStatus::kLeptParseRootNotSingular;
    }
    return ret;
}

LeptType LeptGetType(const LeptJson &v)
{
    return v.type_;
}

double LeptGetNumber(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptNumber);
    return v.n_;
}

