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
const int kLeptParseStringifyInitSize = 256;

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
    kLeptParseMissKey,
    kLeptParseMissColon,
    kLeptParseMissCommaOrCurlyBracket,
};


class LeptMember;

class LeptJson
{
public:
    LeptType type_;
    union
    {
        struct { char *s_; size_t len_; }; //string
        struct { LeptJson* e_; size_t asize_ , capacity_;}; //array
        struct { LeptMember* m_; size_t msize_;}; //member
        double n_;                        //number
    } u;
//    LeptJson() = default;
//    LeptJson(const LeptJson&); //deep copy
//    LeptJson& operator=(LeptJson); //copy opeartor
//    LeptJson(LeptJson&&);  //move copy
//    LeptJson& operator=(LeptJson&&); //move operator
//
//    void Swap(LeptJson& );
};

class LeptMember
{
public:
    char* k_;
    size_t klen_;
    LeptJson v_;
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

size_t LeptGetObjectSize(const LeptJson&);
const char* LeptGetObjectKey(const LeptJson& , size_t);
size_t LeptGetObjectKeyLength(const LeptJson& , size_t);
LeptJson& LeptGetObjectValue(const LeptJson&,  size_t);

void LeptSetArray(LeptJson& , size_t);
size_t LeptGetArrayCapacity(const LeptJson&);
void LeptReserveArray(LeptJson& , size_t);
void LeptShrinkArray(LeptJson&);
LeptJson& LeptPushbackArrayElement(LeptJson&);
void LeptPopbackArrayElement(LeptJson &);
LeptJson& LeptInsertArrayElement(LeptJson& , size_t);
void LeptEraseArrayElement(LeptJson& , size_t , size_t);
void LeptClearArray(LeptJson&);

char* LeptStringify(const LeptJson& ,  size_t&);


const auto kLeptKeyNotExist = ((size_t) - 1);

size_t LeptFindObjectIndex(const LeptJson& , const char* , size_t);
LeptJson& LeptFindObjectValue(const LeptJson&  , const char* , size_t);


bool operator != (const LeptJson& , const LeptJson&);
bool operator == (const LeptJson& , const LeptJson&);


void LeptSetObject(LeptJson& , size_t);
size_t LeptGetObjectCapacity(const LeptJson&);
void LeptReserveObject(LeptJson& , size_t);
void LeptShrinkObejct(LeptJson&);
void LeptClearObejct(LeptJson&);
LeptJson& LeptSetObjectValue(LeptJson&  , const char*  , size_t);
void LeptRemoveObjectValue(LeptJson& , size_t);


#endif //PROJECT_LEPTJSON_H
