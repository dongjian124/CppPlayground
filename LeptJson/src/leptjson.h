//
// Created by sb on 18-9-26.
//

#ifndef PROJECT_LEPTJSON_H
#define PROJECT_LEPTJSON_H

enum class LeptType
{
    kLeptNull , kLeptFalse, kLeptTrue, kLeptNumber, kLeptString, kLeptArray, kLeptObject
};

enum class LeptParseStatus
{
    kLeptParseOk,
    kLeptParseExpectValue,
    kLeptParseInvalidValue,
    kLeptParseRootNotSingular
};


class LeptJson
{
public:
    LeptType type_;
    double n_;
};


LeptParseStatus LeptParse(LeptJson& ,  const char*);

LeptType LeptGetType(const LeptJson &);

double LeptGetNumber(const LeptJson &);


#endif //PROJECT_LEPTJSON_H
