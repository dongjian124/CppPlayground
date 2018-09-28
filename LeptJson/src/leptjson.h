//
// Created by sb on 18-9-26.
//

#ifndef PROJECT_LEPTJSON_H
#define PROJECT_LEPTJSON_H

#include <cstddef>
enum class LeptType
{
    kLeptNull, kLeptFalse, kLeptTrue, kLeptNumber, kLeptString, kLeptArray, kLeptObject
};

const int kLeptParseStackInitSize = 256;

enum class LeptParseStatus
{
    kLeptParseOk,
    kLeptParseExpectValue,
    kLeptParseInvalidValue,
    kLeptParseRootNotSingular,
    kLeptParseNumberTooBig,
    kLeptParseMissQuotationMark,
    kLeptParseInvalidStringEscape,
    kLeptParseInvalidStringChar,
    kLeptParseInvalidUnicodeSurrogate,
    kLeptParseInvalidUnicodeHex,
    kLeptParseMissCommaOrSquareBracket,
};

class LeptJson
{
public:
    LeptType type_;
    union
    {
        struct { char *s_; size_t len_; }; //string
        struct { LeptJson* e_; size_t alen_;}; //array
        double n_;                        //number
    } u;
};

LeptParseStatus LeptParse(LeptJson &, const char *);

LeptType LeptGetType(const LeptJson &);

double LeptGetNumber(const LeptJson &);
void LeptSetNumber(LeptJson&, double);

void LeptFree(LeptJson&);


#define LeptSetNull(v) LeptFree(v)
#define LeptInit(v) do{(v).type_ = LeptType::kLeptNull; } while(0)

int LeptGetBoolean(const LeptJson &);
void LeptSetBoolean(LeptJson& , int);


const char* LeptGetString(const LeptJson&);
size_t LeptGetStringLength(const LeptJson&) ;
void LeptSetString(LeptJson& , const char* , size_t);

size_t LeptGetArraySize(const LeptJson &);
LeptJson& LeptGetArrayElement(const LeptJson&  , size_t);

#endif //PROJECT_LEPTJSON_H
