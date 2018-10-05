//
// Created by sb on 18-9-26.
//

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include "leptjson.h"

#define EXPECT(c, ch) do { assert(*c.json_ == (ch)); c.json_++; } while(0)
#define ISDIGIT(ch)  ((ch >= '0') && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch >= '1') && (ch <= '9'))

#define PUTC(c, ch) do{*(char*)LeptContextPush(c , sizeof(char)) = (ch); } while(0)
#define PUTS(c, s, len) memcpy(LeptContextPush(c , len) , s , len)

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
static LeptParseStatus LeptParseStringRaw(LeptContext &c, char **str, size_t *len);
static LeptParseStatus LeptParseValue(LeptContext &c, LeptJson &v);


std::ostream& operator<<(std::ostream& os , LeptType type)
{
    switch (type)
    {
        case LeptType::kLeptNull:
            os << "LeptNull";
            break;
        case LeptType::kLeptObject:
            os << "LeptObject";
            break;
        case LeptType::kLeptString:
            os << "LeptString";
            break;
        case LeptType::kLeptArray:
            os << "LeptArray";
            break;
        case LeptType::kLeptNumber:
            os << "LeptNumber";
            break;
        case LeptType::kLeptTrue:
            os << "LeptTrue";
            break;
        case LeptType::kLeptFalse:
            os << "LeptFalse";
            break;
    }
    return os;
}

static void LeptParseWhiteSpace(LeptContext &c)
{
    auto p = c.json_;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c.json_ = p;
}

static LeptParseStatus LeptParseNumber(LeptContext &c, LeptJson &v)
{
    auto p = c.json_;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else
    {
        if (!ISDIGIT(*p))
            return LeptParseStatus::kLeptParseInvalidValue;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.')
    {
        p++;
        if (!ISDIGIT(*p))
            return LeptParseStatus::kLeptParseInvalidValue;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E')
    {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p))
            return LeptParseStatus::kLeptParseInvalidValue;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v.u.n_ = strtod(c.json_, nullptr);
    if (errno == ERANGE && (v.u.n_ == HUGE_VAL || v.u.n_ == -HUGE_VAL))
        return LeptParseStatus::kLeptParseNumberTooBig;
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

static LeptParseStatus LeptParseArray(LeptContext &c, LeptJson &v)
{
    size_t size = 0;
    LeptParseStatus ret;
    EXPECT(c, '[');
    LeptParseWhiteSpace(c);
    if (*c.json_ == ']')
    {
        c.json_++;
        LeptSetArray(v, 0);
        return LeptParseStatus::kLeptParseOk;
    }
    for (;;)
    {
        LeptJson e;
        LeptInit(e);
        LeptParseWhiteSpace(c);
        if ((ret = LeptParseValue(c, e)) != LeptParseStatus::kLeptParseOk)
            break;
        memcpy(LeptContextPush(c, sizeof(LeptJson)), &e, sizeof(LeptJson));
        ++size;
        LeptParseWhiteSpace(c);
        if (*c.json_ == ',')
            c.json_++;
        else if (*c.json_ == ']')
        {
            c.json_++;
            LeptSetArray(v, size);
            memcpy(v.u.e_, LeptContextPop(c, size * sizeof(LeptJson)), size * sizeof(LeptJson));
            v.u.asize_ = size;
            return LeptParseStatus::kLeptParseOk;
        } else
        {
            ret = LeptParseStatus::kLeptParseMissCommaOrSquareBracket;
            break;
        }
    }
    for (auto i = 0; i < size; ++i)
    {
        auto tmpv = (LeptJson *) LeptContextPop(c, sizeof(LeptJson));
        LeptFree(*tmpv);
    }
    return ret;
}

static LeptParseStatus LeptParseObject(LeptContext &c, LeptJson &v)
{
    size_t size = 0;
    LeptParseStatus ret;
    EXPECT(c, '{');
    LeptParseWhiteSpace(c);
    if (*c.json_ == '}')
    {
        c.json_++;
        LeptSetObject(v , 0);
        return LeptParseStatus::kLeptParseOk;
    }
    LeptMember m;
    m.k_ = nullptr;
    for (;;)
    {
        char *str;
        LeptInit(m.v_);
        if (*c.json_ != '"')
        {
            ret = LeptParseStatus::kLeptParseMissKey;
            break;
        }
        if ((ret = LeptParseStringRaw(c, &str, &m.klen_)) != LeptParseStatus::kLeptParseOk)
            break;
        memcpy(m.k_ = (char *) malloc(m.klen_ + 1), str, m.klen_ + 1);
        m.k_[m.klen_] = '\0';
        LeptParseWhiteSpace(c);
        if (*c.json_ != ':')
        {
            ret = LeptParseStatus::kLeptParseMissColon;
            break;
        }
        c.json_++;
        LeptParseWhiteSpace(c);
        if ((ret = LeptParseValue(c, m.v_)) != LeptParseStatus::kLeptParseOk)
            break;
        memcpy(LeptContextPush(c, sizeof(LeptMember)), &m, sizeof(LeptMember));
        ++size;
        m.k_ = nullptr;
        LeptParseWhiteSpace(c);
        if (*c.json_ == ',')
        {
            c.json_++;
            LeptParseWhiteSpace(c);
        } else if (*c.json_ == '}')
        {
            c.json_++;
            LeptSetObject(v , size);
            memcpy(v.u.m_ , LeptContextPop(c , size * sizeof(LeptMember)) , size * sizeof(LeptMember));
            v.u.msize_ = size;
            return LeptParseStatus::kLeptParseOk;
        } else
        {
            ret = LeptParseStatus::kLeptParseMissCommaOrCurlyBracket;
            break;
        }
    }
    free(m.k_);
    for (auto i = 0; i < size; ++i)
    {
        auto m = (LeptMember *) LeptContextPop(c, sizeof(LeptMember));
        free(m->k_);
        LeptFree(m->v_);
    }
    v.type_ = LeptType::kLeptNull;
    return ret;
}

static LeptParseStatus LeptParseStringRaw(LeptContext &c, char **str, size_t *len)
{
    size_t head = c.top_;
    EXPECT(c, '\"');
    auto p = c.json_;
    unsigned u, u2;
    for (;;)
    {
        auto ch = *p++;
        switch (ch)
        {
            case '\"':*len = c.top_ - head;
                *str = (char *) LeptContextPop(c, *len);
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

static LeptParseStatus LeptParseString(LeptContext &c, LeptJson &v)
{
    LeptParseStatus ret;
    char *s;
    size_t len;
    if ((ret = LeptParseStringRaw(c, &s, &len)) == LeptParseStatus::kLeptParseOk)
        LeptSetString(v, s, len);
    return ret;
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
        case '{': return LeptParseObject(c, v);
        case '\0': return LeptParseStatus::kLeptParseExpectValue;
    }
}

LeptParseStatus LeptParse(LeptJson &v, const char *json)
{
    LeptContext c;
    c.json_ = json;
    c.stack_ = nullptr;
    c.size_ = c.top_ = 0;
    LeptInit(v);
    LeptParseWhiteSpace(c);
    auto ret = LeptParseValue(c, v);
    if (ret == LeptParseStatus::kLeptParseOk)
    {
        LeptParseWhiteSpace(c);
        if (*c.json_ != '\0')
        {
            v.type_ = LeptType::kLeptNull;
            ret = LeptParseStatus::kLeptParseRootNotSingular;
        }
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
    switch (v.type_)
    {
        case LeptType::kLeptString:
            free(v.u.s_);
            break;
        case LeptType::kLeptArray:
            for (auto i = 0; i < v.u.asize_; ++i)
                LeptFree(v.u.e_[i]);
            free(v.u.e_);
            break;
        case LeptType::kLeptObject:
            for (auto i = 0; i < v.u.msize_; ++i)
            {
                free(v.u.m_[i].k_);
                LeptFree(v.u.m_[i].v_);
            }
            free(v.u.m_);
            break;
        default: break;
    }
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
    return v.u.asize_;
}

LeptJson &LeptGetArrayElement(const LeptJson &v, size_t index)
{
    assert(v.type_ == LeptType::kLeptArray);
    assert(v.u.asize_ > index);
    return v.u.e_[index];
}

size_t LeptGetObjectSize(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptObject);
    return v.u.msize_;
}

const char *LeptGetObjectKey(const LeptJson &v, size_t index)
{
    assert(v.type_ == LeptType::kLeptObject);
    assert(v.u.msize_ > index);
    return v.u.m_[index].k_;
}

size_t LeptGetObjectKeyLength(const LeptJson &v, size_t index)
{
    assert(v.type_ == LeptType::kLeptObject);
    assert(v.u.msize_ > index);
    return v.u.m_[index].klen_;
}

LeptJson &LeptGetObjectValue(const LeptJson &v, size_t index)
{
    assert(v.type_ == LeptType::kLeptObject);
    assert(index < v.u.msize_);
    return v.u.m_[index].v_;
}

static void LeptStringifyString(LeptContext &c, const char *s, size_t len)
{
    static const char kHexDigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    size_t size;
    char *head, *p;
    assert(s != nullptr);
    p = head = (char *) LeptContextPush(c, size = len * 6 + 2);
    *p++ = '"';
    for (auto i = 0; i < len; ++i)
    {
        unsigned char ch = (unsigned char) s[i];
        switch (ch)
        {
            case '\"':*p++ = '\\';
                *p++ = '\"';
                break;
            case '\\':*p++ = '\\';
                *p++ = '\\';
                break;
            case '\b':*p++ = '\\';
                *p++ = 'b';
                break;
            case '\f':*p++ = '\\';
                *p++ = 'f';
                break;
            case '\n':*p++ = '\\';
                *p++ = 'n';
                break;
            case '\t':*p++ = '\\';
                *p++ = 't';
                break;
            case '\r':*p++ = '\\';
                *p++ = 'r';
                break;
            default:
                if (ch < 0x20)
                {
                    *p++ = '\\';
                    *p++ = 'u';
                    *p++ = '0';
                    *p++ = '0';
                    *p++ = kHexDigits[ch >> 4];
                    *p++ = kHexDigits[ch & 15];
                } else
                    *p++ = s[i];
        }
    }
    *p++ = '"';
    c.top_ -= size - (p - head);
}

static void LeptStringifyValue(LeptContext &c, const LeptJson &v)
{
    switch (v.type_)
    {
        case LeptType::kLeptNull:PUTS(c, "null", 4);
            break;
        case LeptType::kLeptFalse:PUTS(c, "false", 5);
            break;
        case LeptType::kLeptTrue:PUTS(c, "true", 4);
            break;
        case LeptType::kLeptNumber:c.top_ -= 32 - sprintf((char *) LeptContextPush(c, 32), "%.17g", v.u.n_);
            break;
        case LeptType::kLeptString:LeptStringifyString(c, v.u.s_, v.u.len_);
            break;
        case LeptType::kLeptArray:PUTC(c, '[');
            for (auto i = 0; i < v.u.asize_; ++i)
            {
                if (i > 0)
                    PUTC(c, ',');
                LeptStringifyValue(c, v.u.e_[i]);
            }
            PUTC(c, ']');
            break;
        case LeptType::kLeptObject:PUTC(c, '{');
            for (auto i = 0; i < v.u.msize_; ++i)
            {
                if (i > 0)
                    PUTC(c, ',');
                LeptStringifyString(c, v.u.m_[i].k_, v.u.m_[i].klen_);
                PUTC(c, ':');
                LeptStringifyValue(c, v.u.m_[i].v_);
            }
            PUTC(c, '}');
            break;
        default:assert(0 && "invalid type");
    }
}

char *LeptStringify(const LeptJson &v, size_t &len)
{
    LeptContext c;
    c.stack_ = (char *) malloc(c.size_ = kLeptParseStringifyInitSize);
    c.top_ = 0;
    LeptStringifyValue(c, v);
    len = c.top_;
    PUTC(c, '\0');
    return c.stack_;
}

size_t LeptFindObjectIndex(const LeptJson &v, const char *key, size_t klen)
{
    size_t i;
    assert(key != nullptr);
    assert(v.type_ == LeptType::kLeptObject);
    for (i = 0; i < v.u.msize_; ++i)
    {
        if (v.u.m_[i].klen_ == klen && memcmp(v.u.m_[i].k_, key, klen) == 0)
            return i;
    }
    return kLeptKeyNotExist;
}

LeptJson &LeptFindObjectValue(const LeptJson &v, const char *key, size_t klen)
{
    auto index = LeptFindObjectIndex(v, key, klen);
    return index != kLeptKeyNotExist ? v.u.m_[index].v_ : *(LeptJson *) nullptr;
}

bool operator==(const LeptJson &lhs, const LeptJson &rhs)
{
    if (lhs.type_ != rhs.type_)
        return false;
    switch (lhs.type_)
    {
        case LeptType::kLeptString:return lhs.u.len_ == rhs.u.len_ && memcmp(lhs.u.s_, rhs.u.s_, lhs.u.len_) == 0;
        case LeptType::kLeptNumber:return lhs.u.n_ == rhs.u.n_;
        case LeptType::kLeptArray:
            if (lhs.u.asize_ != rhs.u.asize_)
                return false;
            for (auto i = 0; i < lhs.u.asize_; ++i)
                if (lhs.u.e_[i] != rhs.u.e_[i])
                    return false;
            break;
        case LeptType::kLeptObject:
            if (lhs.u.msize_ != rhs.u.msize_)
                return false;
            for (auto i = 0; i < lhs.u.msize_; ++i)
            {
                auto ridx = LeptFindObjectIndex(rhs, lhs.u.m_[i].k_, lhs.u.m_[i].klen_);
                if (ridx == kLeptKeyNotExist)
                    return false;
                if (lhs.u.m_[i].v_ != rhs.u.m_[ridx].v_)
                    return false;
            }
            break;
    }
    return true;
}

bool operator!=(const LeptJson &lhs, const LeptJson &rhs)
{
    return !(lhs == rhs);
}

void LeptSetType(LeptJson& v , LeptType type)
{
    v.type_ = type;
}

LeptJson::LeptJson(const LeptJson &rhs)
{
//    LeptInit(*this);  //FIXME: It is right?
    switch (rhs.type_)
    {
        case LeptType::kLeptString:
            LeptSetString(*this, rhs.u.s_, rhs.u.len_);
            break;
        case LeptType::kLeptArray: //deep copy
            LeptSetArray(*this , rhs.u.acapacity_);
            for(auto i = 0 ; i < rhs.u.asize_; ++i)
            {
                LeptInit(u.e_[i]);
                u.e_[i] = LeptJson(rhs.u.e_[i]);
            }
            u.asize_ = rhs.u.asize_;
            break;
        case LeptType::kLeptObject:
            LeptSetObject(*this, rhs.u.mcapacity_);
            for(int i = 0; i < rhs.u.msize_; ++i)
            {
                if(rhs.u.m_[i].k_ != nullptr)
                {
                    memcpy(u.m_[i].k_ = (char *) malloc(rhs.u.m_[i].klen_ + 1), rhs.u.m_[i].k_, rhs.u.m_[i].klen_);
                    u.m_[i].klen_ = rhs.u.m_[i].klen_;
                    u.m_[i].k_[u.m_[i].klen_] = '\0';
                }
                LeptInit(u.m_[i].v_);
                u.m_[i].v_ = LeptJson(rhs.u.m_[i].v_);
            }
            u.msize_ = rhs.u.msize_;
            break;
        case LeptType::kLeptNumber:
            LeptSetNumber(*this, rhs.u.n_);
            break;
        case LeptType::kLeptFalse:  case LeptType::kLeptTrue:
            LeptSetBoolean(*this, rhs.type_ == LeptType::kLeptTrue ? 1 : 0);
            break;
        case LeptType::kLeptNull:
            LeptSetNull(*this);
            break;
    }
}


LeptJson& LeptJson::operator=(LeptJson rhs)
{
    swap(rhs);
    return *this;
}

LeptJson::LeptJson(LeptJson && rhs)
{
    LeptFree(*this);

    switch (rhs.type_)
    {
        case LeptType::kLeptFalse:
        case LeptType::kLeptTrue:
            LeptSetBoolean(*this , rhs.type_ == LeptType::kLeptTrue ? 1 : 0);
            break;
        case LeptType::kLeptNull:
            LeptSetNull(*this);
        case LeptType::kLeptArray:
            u.e_ = rhs.u.e_;
            u.asize_ = rhs.u.asize_;
            u.acapacity_ = rhs.u.acapacity_;
            rhs.u.e_ = nullptr;
            rhs.u.asize_ = rhs.u.acapacity_ = 0;
            type_ = rhs.type_;
            break;
        case LeptType::kLeptObject:
            u.msize_ = rhs.u.msize_;
            u.mcapacity_ = rhs.u.mcapacity_;
            u.m_ = rhs.u.m_;
            rhs.u.m_ = nullptr;
            rhs.u.msize_ = rhs.u.mcapacity_ = 0;
            type_ = rhs.type_;
            break;
        case LeptType::kLeptNumber:
            LeptSetNumber(*this , rhs.u.n_);
            break;
        case LeptType::kLeptString:
            u.s_ = rhs.u.s_;
            u.len_ = rhs.u.len_;
            rhs.u.s_ = nullptr;
            rhs.u.len_ = 0;
            type_ = rhs.type_;
            break;
    }

    LeptInit(rhs);
}




void LeptJson::swap(LeptJson & rhs)
{
    if(*this != rhs)
    {
        if(type_ == LeptType::kLeptNull || type_ == LeptType::kLeptFalse || type_ == LeptType::kLeptTrue)
        {
            if(rhs.type_ == LeptType::kLeptObject)
            {
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
            }
            else if(rhs.type_ == LeptType::kLeptArray)
            {
                std::swap(u.e_ , rhs.u.e_);
                std::swap(u.asize_ , rhs.u.asize_);
            }
            else if(rhs.type_ == LeptType::kLeptString)
            {
                std::swap(u.s_ , rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
            }
            else if(rhs.type_ == LeptType::kLeptNumber)
            {
                std::swap(u.n_ , rhs.u.n_);
            }
        }
        else if(type_ == LeptType::kLeptObject)
        {
            if(rhs.type_ == LeptType::kLeptNull  || rhs.type_ == LeptType::kLeptTrue || rhs.type_ == LeptType::kLeptFalse)
            {
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
            }
            else if(rhs.type_ == LeptType::kLeptObject)
            {
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
            }
            else if(rhs.type_ == LeptType::kLeptArray)
            {
                auto tmp = rhs.u.e_;
                auto tmpsize = rhs.u.asize_;
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
                u.e_ = tmp;
                u.asize_ = tmpsize;
            }
            else if(rhs.type_ == LeptType::kLeptNumber)
            {
                auto tmp = rhs.u.n_;
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
                u.n_ = tmp;
            }
            else if(rhs.type_ == LeptType::kLeptString)
            {
                auto tmp = rhs.u.s_;
                auto tmpsize = rhs.u.len_;
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
                u.s_ = tmp;
                u.len_ = tmpsize;
            }
        }
        else if(type_ == LeptType::kLeptArray)
        {
            if(rhs.type_ == LeptType::kLeptNull || rhs.type_ == LeptType::kLeptTrue || rhs.type_ == LeptType::kLeptFalse)
            {
                std::swap(u.e_ , rhs.u.e_);
                std::swap(u.asize_ , rhs.u.asize_);
            }
            else if(rhs.type_ == LeptType::kLeptObject)
            {
                auto tmp = rhs.u.m_;
                auto tmpsize = rhs.u.msize_;
                std::swap(u.e_ , rhs.u.e_);
                std::swap(u.asize_, rhs.u.asize_);
                u.m_ = tmp;
                u.msize_ = tmpsize;
            }
            else if(rhs.type_ == LeptType::kLeptArray)
            {
                std::swap(u.e_ , rhs.u.e_);
                std::swap(u.asize_ , rhs.u.asize_);
            }
            else if(rhs.type_ == LeptType::kLeptNumber)
            {
                auto tmp = rhs.u.n_;
                std::swap(u.e_ , rhs.u.e_);
                std::swap(u.asize_ , rhs.u.asize_);
                u.n_ = tmp;
            }
            else if(rhs.type_ == LeptType::kLeptString)
            {
                auto tmp = rhs.u.s_;
                auto tmpsize = rhs.u.len_;
                std::swap(u.e_ , rhs.u.e_);
                std::swap(u.asize_ , rhs.u.asize_);
                u.s_ = tmp;
                u.len_ = tmpsize;
            }
        }
        else if(type_ == LeptType::kLeptNumber)
        {
            if(rhs.type_ == LeptType::kLeptTrue || rhs.type_ == LeptType::kLeptFalse || rhs .type_ == LeptType::kLeptNull)
            {
                std::swap(u.n_ , rhs.u.n_);
            }
            else if(rhs.type_ == LeptType::kLeptObject)
            {
                auto tmp = u.n_;
                std::swap(u.m_ , rhs.u.m_);
                std::swap(u.msize_ , rhs.u.msize_);
                rhs.u.n_ = tmp;
            }
            else if(rhs.type_ == LeptType::kLeptArray)
            {
                auto tmp = u.n_;
                std::swap(u.e_ ,  rhs.u.e_);
                std::swap(u.asize_ , rhs.u.asize_);
                rhs.u.n_ = tmp;
            }
            else if(rhs.type_ == LeptType::kLeptNumber)
            {
                std::swap(u.n_ , rhs.u.n_);
            }
            else if(rhs.type_ == LeptType::kLeptString)
            {
                auto tmp = u.n_;
                std::swap(u.s_ , rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
            }
        }
        else if(type_ == LeptType::kLeptString)
        {
            if(rhs.type_ == LeptType::kLeptTrue || rhs.type_ == LeptType::kLeptFalse || rhs.type_ == LeptType::kLeptNull)
            {
                std::swap(u.s_, rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
            }
            else if(rhs.type_ == LeptType::kLeptNumber)
            {
                auto tmp = rhs.u.n_;
                std::swap(u.s_ , rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
                u.n_ = tmp;
            }
            else if(rhs.type_ == LeptType::kLeptObject)
            {
                auto tmp = rhs.u.m_;
                auto tmpsize = rhs.u.msize_;
                std::swap(u.s_ , rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
                u.msize_ = tmpsize;
                u.m_ = tmp;
            }
            else if(rhs.type_ == LeptType::kLeptArray)
            {
                auto tmp = rhs.u.e_;
                auto tmpsize = rhs.u.asize_;
                std::swap(u.s_ , rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
                u.e_ = tmp;
                u.asize_ = tmpsize;
            }
            else if(rhs.type_ == LeptType::kLeptString)
            {
                std::swap(u.s_ , rhs.u.s_);
                std::swap(u.len_ , rhs.u.len_);
            }
        }
        std::swap(type_ , rhs.type_);
    }
}

void LeptSetArray(LeptJson &v, size_t cap)
{
    LeptFree(v);
    v.type_ = LeptType::kLeptArray;
    v.u.asize_ = 0;
    v.u.acapacity_ = cap;
    v.u.e_ = cap > 0 ? (LeptJson *) malloc(cap * sizeof(LeptJson)) : nullptr;
}

size_t LeptGetArrayCapacity(const LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptArray);
    return v.u.acapacity_;
}

void LeptReserveArray(LeptJson &v, size_t cap)
{
    assert(v.type_ == LeptType::kLeptArray);
    if (v.u.acapacity_ < cap)
    {
        v.u.acapacity_ = cap;
        v.u.e_ = (LeptJson *) realloc(v.u.e_, cap * sizeof(LeptJson));
    }
}

void LeptShrinkArray(LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptArray);
    if (v.u.acapacity_ > v.u.asize_)
    {
        v.u.acapacity_ = v.u.asize_;
        v.u.e_ = (LeptJson *) realloc(v.u.e_, v.u.acapacity_ * sizeof(LeptJson));
    }
}

LeptJson &LeptPushbackArrayElement(LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptArray);
    if (v.u.asize_ == v.u.acapacity_)
        LeptReserveArray(v, v.u.acapacity_ == 0 ? 1 : v.u.acapacity_ * 2);
    LeptInit(v.u.e_[v.u.asize_]);
    return v.u.e_[v.u.asize_++];
}

void LeptPopbackArrayElement(LeptJson &v)
{
    assert(v.type_ == LeptType::kLeptArray && v.u.asize_ > 0);
    LeptFree(v.u.e_[--v.u.asize_]);
}

void LeptClearArray(LeptJson& v)
{
    assert(v.type_ == LeptType::kLeptArray);
    LeptFree(v);
    v.u.e_ = nullptr;
    v.u.asize_ = 0;
}


void LeptSetObject(LeptJson& v, size_t cap)
{
    LeptFree(v);
    v.type_ = LeptType::kLeptObject;
    v.u.msize_ = 0;
    v.u.mcapacity_ = cap;
    v.u.m_ = cap > 0 ? (LeptMember*) malloc(cap * sizeof(LeptMember)) : nullptr;
}

size_t LeptGetObjectCapacity(const LeptJson& v)
{
    assert(v.type_ == LeptType::kLeptObject);
    return v.u.mcapacity_;
}

void LeptReserveObject(LeptJson& v , size_t cap)
{
    assert(v.type_ == LeptType::kLeptObject);
    if(cap > v.u.mcapacity_)
    {
        v.u.mcapacity_ = cap;
        v.u.m_ = (LeptMember*)realloc(v.u.e_ , cap * sizeof(LeptMember));
    }
}

void LeptShrinkObejct(LeptJson& v)
{
    assert(v.type_ == LeptType::kLeptObject);
    if(v.u.mcapacity_ > v.u.msize_)
    {
        v.u.mcapacity_ = v.u.msize_;
        v.u.m_ = (LeptMember*)realloc(v.u.m_ , v.u.mcapacity_ * sizeof(LeptMember));
    }
}

void LeptClearObejct(LeptJson& v)
{
    assert(v.type_ == LeptType::kLeptObject);
    LeptFree(v);
    v.u.msize_ = 0;
    v.u.m_ = nullptr;
}

LeptJson& LeptSetObjectValue(LeptJson& v , const char* key, size_t klen)
{
    assert(v.type_ == LeptType::kLeptObject);
    auto if_exist_idx = LeptFindObjectIndex(v , key , klen);
    if(if_exist_idx == kLeptKeyNotExist)
    {
        if(v.u.msize_ == v.u.mcapacity_)
            LeptReserveObject(v , v.u.mcapacity_ == 0 ? 1 : v.u.mcapacity_ * 2);
        LeptInit(v.u.m_[v.u.msize_].v_);
        return v.u.m_[v.u.msize_++].v_;
    }
    else
        return v.u.m_[if_exist_idx].v_;
}


