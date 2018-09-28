//
// Created by sb on 18-9-26.
//

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <cstring>
#include "leptjson.h"

#define EXPECT(c, ch) do { assert(*c.json_ == (ch)); c.json_++; } while(0)
#define ISDIGIT(ch)  ((ch >= '0') && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch >= '1') && (ch <= '9'))

#define PUTC(c, ch) do{*(char*)LeptContextPush(c , sizeof(char)) = (ch); } while(0)

class LeptContext
{
public:
    const char *json_;
    char *stack_;
    size_t size_, top_;
};

static void *LeptContextPop(LeptContext &, size_t);
static void *LeptContextPush(LeptContext &, size_t);
void LeptFree(LeptJson &v);
static LeptParseStatus LeptParseValue(LeptContext &, LeptJson &);

static void LeptParseWhiteSpace(LeptContext &c)
{
    auto p = c.json_;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c.json_ = p;
}

static LeptParseStatus LeptParseNumber(LeptContext &c, LeptJson &v)
{
    const char *p = c.json_;
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
        if (!ISDIGIT(*p))
            return LeptParseStatus::kLeptParseInvalidValue;
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
    v.u.n_ = strtod(c.json_, nullptr);
    if (errno == ERANGE && (v.u.n_ == HUGE_VAL || v.u.n_ == -HUGE_VAL))
        return LeptParseStatus::kLeptParseInvalidValue;
    v.type_ = LeptType::kLeptNumber;
    c.json_ = p;
    return LeptParseStatus::kLeptParseOk;
}

static LeptParseStatus LeptParseLiterla(LeptContext &c, LeptJson &v, const char *literla, LeptType type)
{
    size_t i;
    EXPECT(c, literla[0]);
    for (i = 0; literla[i + 1]; ++i)
    {
        if (c.json_[i] != literla[i + 1])
            return LeptParseStatus::kLeptParseInvalidValue;
    }
    c.json_ += i;
    v.type_ = type;
    return LeptParseStatus::kLeptParseOk;
}

static const char *LeptParseHex4(const char *p, unsigned &u)
{
    u = 0;
    for (auto i = 0; i < 4; ++i)
    {
        auto ch = *p++;
        u <<= 4;
        if (ch >= '0' && ch <= '9') u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F') u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f') u |= ch - ('a' - 10);
        else
            return nullptr;
    }
    return p;
}
static void LeptEncodeUTF8(LeptContext &c, unsigned u)
{
    if (u <= 0x7F)
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF)
    {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF)
    {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    } else
    {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
}

#define STRING_ERROR(ret) do {c.top_ = head; return ret;} while(0)

static LeptParseStatus LeptParseString(LeptContext &c, LeptJson &v)
{
    size_t head = c.top_, len;
    EXPECT(c, '\"');
    auto p = c.json_;
    unsigned u, u2;
    for (;;)
    {
        auto ch = *p++;
        switch (ch)
        {
            case '\"':len = c.top_ - head;
                LeptSetString(v, (const char *) LeptContextPop(c, len), len);
                c.json_ = p;
                return LeptParseStatus::kLeptParseOk;
            case '\0':c.top_ = head;
                return LeptParseStatus::kLeptParseMissQuotationMark;
            case '\\':
                switch (*p++)
                {
                    case 'u':
                        if (!(p = LeptParseHex4(p, u)))
                            STRING_ERROR(LeptParseStatus::kLeptParseInvalidUnicodeHex);
                        if (u >= 0xD800 && u <= 0xDBFF)  //代理对
                        {
                            if (*p++ != '\\')
                                STRING_ERROR(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate);
                            if (*p++ != 'u')
                                STRING_ERROR(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate);
                            if (!(p = LeptParseHex4(p, u2)))
                                STRING_ERROR(LeptParseStatus::kLeptParseInvalidUnicodeHex);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        LeptEncodeUTF8(c, u);
                        break;
                    case '\"':PUTC(c, '\"');
                        break;
                    case '\\':PUTC(c, '\\');
                        break;
                    case '/':PUTC(c, '/');
                        break;
                    case 'b':PUTC(c, '\b');
                        break;
                    case 'f':PUTC(c, '\f');
                        break;
                    case 'n':PUTC(c, '\n');
                        break;
                    case 'r':PUTC(c, '\r');
                        break;
                    case 't':PUTC(c, '\t');
                        break;
                    default:STRING_ERROR(LeptParseStatus::kLeptParseInvalidStringEscape);
                }
                break;
            default:
                if ((unsigned char) ch < 0x20)
                    STRING_ERROR(LeptParseStatus::kLeptParseInvalidStringChar);
                PUTC(c, ch);
        }
    }
}

static LeptParseStatus LeptParseArray(LeptContext &c, LeptJson &v)
{
    size_t size = 0;
    LeptParseStatus ret;
    EXPECT(c, '[');
    LeptParseWhiteSpace(c);
    if (*c.json_ == ']')
    {
        c.json_++;
        v.type_ = LeptType::kLeptArray;
        v.u.alen_ = 0;
        v.u.e_ = nullptr;
        return LeptParseStatus::kLeptParseOk;
    }
    for (;;)
    {
        LeptJson e;
        LeptInit(e);
        LeptParseWhiteSpace(c);
        if ((ret = LeptParseValue(c, e)) != LeptParseStatus::kLeptParseOk)
            return ret;
        memcpy(LeptContextPush(c, sizeof(LeptJson)), &e, sizeof(LeptJson));
        ++size;
        LeptParseWhiteSpace(c);
        if (*c.json_ == ',')
            c.json_++;
        else if (*c.json_ == ']')
        {
            c.json_++;
            v.type_ = LeptType::kLeptArray;
            v.u.alen_ = size;
            size *= sizeof(LeptJson);
            memcpy(v.u.e_ = (LeptJson *) malloc(size), LeptContextPop(c, size), size);
            return LeptParseStatus::kLeptParseOk;
        }
        else
            return LeptParseStatus::kLeptParseMissCommaOrSquareBracket;
    }
}

static LeptParseStatus LeptParseValue(LeptContext &c, LeptJson &v)
{
    switch (*c.json_)
    {
        case 'n': return LeptParseLiterla(c, v, "null", LeptType::kLeptNull);
        case 't': return LeptParseLiterla(c, v, "true", LeptType::kLeptTrue);
        case 'f': return LeptParseLiterla(c, v, "false", LeptType::kLeptFalse);
        default: return LeptParseNumber(c, v);
        case '"': return LeptParseString(c, v);
        case '[': return LeptParseArray(c, v);
        case '\0': return LeptParseStatus::kLeptParseExpectValue;
    }
}

LeptParseStatus LeptParse(LeptJson &v, const char *json)
{
    LeptContext c;
    c.json_ = json;
    c.stack_ = nullptr;
    c.size_ = c.top_ = 0;
    v.type_ = LeptType::kLeptNull;
    LeptParseWhiteSpace(c);
    auto ret = LeptParseValue(c, v);
    if (ret == LeptParseStatus::kLeptParseOk)
    {
        LeptParseWhiteSpace(c);
        if (*c.json_ != '\0')
            return LeptParseStatus::kLeptParseRootNotSingular;
    }
    assert(c.top_ == 0);
    free(c.stack_);
    return ret;
}

LeptType LeptGetType(const LeptJson &v)
{
    return v.type_;
}

double LeptGetNumber(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptNumber);
    return v.u.n_;
}

void LeptSetNumber(LeptJson &v, double n)
{
    LeptFree(v);
    v.u.n_ = n;
    v.type_ = LeptType::kLeptNumber;
}

void LeptFree(LeptJson &v)
{
    if (v.type_ == LeptType::kLeptString)
        free(v.u.s_);
    v.type_ = LeptType::kLeptNull;
}

//json规范可以允许有\0
void LeptSetString(LeptJson &v, const char *s, size_t len)
{
    assert(s != nullptr || len == 0);
    LeptFree(v);
    v.u.s_ = (char *) malloc(len + 1);
    memcpy(v.u.s_, s, len);
    v.u.s_[len] = '\0';
    v.u.len_ = len;
    v.type_ = LeptType::kLeptString;
}

const char *LeptGetString(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptString);
    return v.u.s_;
}

size_t LeptGetStringLength(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptString);
    return v.u.len_;
}

int LeptGetBoolean(const LeptJson &v)
{
    assert(v.type_ != LeptType::kLeptTrue || v.type_ != LeptType::kLeptFalse);
    return v.type_ == LeptType::kLeptTrue;
}

void LeptSetBoolean(LeptJson &v, int ok)
{
    assert(v.type_ != LeptType::kLeptTrue || v.type_ != LeptType::kLeptFalse);
    LeptFree(v);
    v.type_ = ok ? LeptType::kLeptTrue : LeptType::kLeptFalse;
}

static void *LeptContextPush(LeptContext &c, size_t size)
{
    void *ret;
    assert(size > 0);
    if (c.top_ + size >= c.size_)
    {
        if (c.size_ == 0)
            c.size_ = kLeptParseStackInitSize;
        while (c.top_ + size >= c.size_)
            c.size_ += c.size_ >> 1;
        c.stack_ = (char *) realloc(c.stack_, c.size_);
    }
    ret = c.stack_ + c.top_;
    c.top_ += size;
    return ret;
}

static void *LeptContextPop(LeptContext &c, size_t size)
{
    assert(c.top_ >= size);
    return c.stack_ + (c.top_ -= size);
}

size_t LeptGetArraySize(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptArray);
    return v.u.alen_;
}

LeptJson &LeptGetArrayElement(const LeptJson &v, size_t index)
{
    assert(v.type_ == LeptType::kLeptArray);
    assert(v.u.alen_ > index);
    return v.u.e_[index];
}


