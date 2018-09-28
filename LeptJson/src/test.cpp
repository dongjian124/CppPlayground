
#include "leptjson.h"
#include <gtest/gtest.h>

using namespace std;

class TestParse : public ::testing::Test
{

protected:
    void SetUp() override
    {
        null_.type_ = LeptType::kLeptTrue;
        expect_value_.type_ = LeptType::kLeptFalse;
        true_.type_ = LeptType::kLeptFalse;
        false_.type_ = LeptType::kLeptTrue;
        string_.type_ = LeptType::kLeptFalse;
    }

    LeptJson null_;
    LeptJson expect_value_;
    LeptJson true_;
    LeptJson false_;
    LeptJson string_;
};


//test parse
TEST_F(TestParse, test_parse)
{
    // null
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(TestParse::null_, "null"));
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(TestParse::null_));

    //Expect vale
    EXPECT_EQ(LeptParseStatus::kLeptParseExpectValue, LeptParse(TestParse::expect_value_, ""));
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(TestParse::expect_value_));
    TestParse::expect_value_.type_ = LeptType::kLeptFalse;
    EXPECT_EQ(LeptParseStatus::kLeptParseExpectValue, LeptParse(TestParse::expect_value_, " "));
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(TestParse::expect_value_));

    //true
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(TestParse::true_, "true"));
    EXPECT_EQ(LeptType::kLeptTrue, LeptGetType(TestParse::true_));

    //false
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(TestParse::false_, "false"));
    EXPECT_EQ(LeptType::kLeptFalse, LeptGetType(TestParse::false_));
}

//test number
class TestNumber : public ::testing::TestWithParam<pair<double, char *>>
{
};

TEST_P(TestNumber, test_right_number)
{
    auto it = GetParam();
    LeptJson v;
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, it.second));
    EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(v));
    EXPECT_DOUBLE_EQ(it.first, LeptGetNumber(v));
}

INSTANTIATE_TEST_CASE_P(MyTestNumber, TestNumber, ::testing::Values
    (
        pair<double, char *>(0.0, "0"),
        pair<double, char *>{0.0, "-0"},
        pair<double, char *>{0.0, "-0.0"},
        pair<double, char *>{1.0, "1"},
        pair<double, char *>{-1.0, "-1"},
        pair<double, char *>{1.5, "1.5"},
        pair<double, char *>{-1.5, "-1.5"},
        pair<double, char *>{3.1416, "3.1416"},
        pair<double, char *>{1E10, "1E10"},
        pair<double, char *>{1e10, "1e10"},
        pair<double, char *>{1E+10, "1E+10"},
        pair<double, char *>{1E-10, "1E-10"},
        pair<double, char *>{-1E10, "-1E10"},
        pair<double, char *>{-1e10, "-1e10"},
        pair<double, char *>{-1E+10, "-1E+10"},
        pair<double, char *>{-1E-10, "-1E-10"},
        pair<double, char *>{1.234E+10, "1.234E+10"},
        pair<double, char *>{1.234E-10, "1.234E-10"},
        pair<double, char *>{0.0, "1e-10000"},
        pair<double, char *>{1.0000000000000002, "1.0000000000000002"},
        pair<double, char *>{4.9406564584124654e-324, "4.9406564584124654e-324"},
        pair<double, char *>{-4.9406564584124654e-324, "-4.9406564584124654e-324"},
        pair<double, char *>{2.2250738585072009e-308, "2.2250738585072009e-308"},
        pair<double, char *>{-2.2250738585072009e-308, "-2.2250738585072009e-308"},
        pair<double, char *>{2.2250738585072014e-308, "2.2250738585072014e-308"},
        pair<double, char *>{-2.2250738585072014e-308, "-2.2250738585072014e-308"},
        pair<double, char *>{1.7976931348623157e+308, "1.7976931348623157e+308"},
        pair<double, char *>{-1.7976931348623157e+308, "-1.7976931348623157e+308"}
    ));

class TestInvalid : public ::testing::TestWithParam<const char *>
{
};

TEST_P(TestInvalid, test_wrong_number)
{
    auto it = GetParam();
    LeptJson v;
    v.type_ = LeptType::kLeptFalse;
    EXPECT_EQ(LeptParseStatus::kLeptParseInvalidValue, LeptParse(v, it)) << "wrong number is: " << it;
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(v)) << "wrong number is: " << it;
}

INSTANTIATE_TEST_CASE_P(MyTestWrongNumber, TestInvalid, ::testing::Values
    ("+0", "+1", ".123", "1.", "INF", "inf", "NAN", "nan", "[1, ]", "[\"a\" , nul]"));

class TestAccessString : public ::testing::Test
{

    void SetUp() override
    {
        LeptInit(v);
    }

    void TearDown() override
    {
        LeptFree(v);
    }

public:
    LeptJson v;
};

TEST_F(TestAccessString, test_access_string)
{
    LeptSetString(v, "", 0);
    EXPECT_STREQ("", LeptGetString(v));
    LeptSetString(v, "Hello", 5);
    EXPECT_STREQ("Hello", LeptGetString(v));
}

TEST_F(TestAccessString, test_access_boolean)
{
    LeptSetString(v, "a", 1);
    LeptSetBoolean(v, 1);
    EXPECT_TRUE(LeptGetBoolean(v));
    LeptSetBoolean(v, 0);
    EXPECT_FALSE(LeptGetBoolean(v));
}

TEST_F(TestAccessString, test_access_number)
{
    LeptSetString(v, "a", 1);
    LeptSetNumber(v, 1.234);
    EXPECT_DOUBLE_EQ(1.234, LeptGetNumber(v));
}

class TestString : public ::testing::TestWithParam<pair<const char *, const char *>>
{
public:
    LeptJson v;

    void SetUp() override
    {
        LeptInit(v);
    }

    void TearDown() override
    {
        LeptFree(v);
    }
};

TEST_P(TestString, test_string)
{
    auto it = GetParam();
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, it.second));
    EXPECT_EQ(LeptType::kLeptString, LeptGetType(v));
    EXPECT_STREQ(it.first, LeptGetString(v));
}

INSTANTIATE_TEST_CASE_P(MyTestString, TestString, ::testing::Values(
    pair<const char *, const char *>("", "\"\""),
    pair<const char *, const char *>("Hello", "\"Hello\""),
    pair<const char *, const char *>("Hello\nWorld", "\"Hello\\nWorld\""),
    pair<const char *, const char *>("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\""),
    pair<const char *, const char *>("Hello\0World", "\"Hello\\u0000World\""),
    pair<const char *, const char *>("\x24", "\"\\u0024\""),
    pair<const char *, const char *>("\xC2\xA2", "\"\\u00A2\""),
    pair<const char *, const char *>("\xE2\x82\xAC", "\"\\u20AC\""),
    pair<const char *, const char *>("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""),
    pair<const char *, const char *>("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"")
));

class TestArray : public ::testing::Test
{
public:

    LeptJson v;

    void SetUp() override
    {
        LeptInit(v);
    }

    void TearDown() override
    {
        LeptFree(v);
    }
};

TEST_F(TestArray, test_array)
{
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, "[ ]"));
    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(v));
    EXPECT_EQ(0, LeptGetArraySize(v));
    LeptFree(v);

    LeptInit(v);
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, "[ null , false ,true , 123 , \"abc\" ]"));
    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(v));
    EXPECT_EQ(5, LeptGetArraySize(v));
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(LeptGetArrayElement(v, 0)));
    EXPECT_EQ(LeptType::kLeptFalse, LeptGetType(LeptGetArrayElement(v, 1)));
    EXPECT_EQ(LeptType::kLeptTrue, LeptGetType(LeptGetArrayElement(v, 2)));
    EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(LeptGetArrayElement(v, 3)));
    EXPECT_EQ(LeptType::kLeptString, LeptGetType(LeptGetArrayElement(v, 4)));
    EXPECT_DOUBLE_EQ(123.0, LeptGetNumber(LeptGetArrayElement(v, 3)));
    EXPECT_STREQ("abc", LeptGetString(LeptGetArrayElement(v, 4)));
    LeptFree(v);

    LeptInit(v);
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, "[ [ ] , [ 0 ] , [ 0  , 1 ] , [ 0 , 1, 2 ] ]"));
    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(v));
    EXPECT_EQ(4, LeptGetArraySize(v));
    for (auto i = 0; i < 4; ++i)
    {
        auto a = LeptGetArrayElement(v, i);
        EXPECT_EQ(LeptType::kLeptArray, LeptGetType(a));
        EXPECT_EQ(i, LeptGetArraySize(a));
        for (auto j = 0; j < i; ++j)
        {
            auto e = LeptGetArrayElement(a, j);
            EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(e));
            EXPECT_EQ((double) j, LeptGetNumber(e));
        }
    }
}

class TestObject : public ::testing::Test
{

public:
    LeptJson v;

    void SetUp() override
    {
        LeptInit(v);
    }

    void TearDown() override
    {
        LeptFree(v);
    }
};

TEST_F(TestObject, test_object)
{
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, " { } "));
    EXPECT_EQ(LeptType::kLeptObject, LeptGetType(v));
    EXPECT_EQ(0, LeptGetObjectSize(v));
    LeptFree(v);

    LeptInit(v);
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v,
                                                       " { "
                                                       "\"n\" : null , "
                                                       "\"f\" : false , "
                                                       "\"t\" : true , "
                                                       "\"i\" : 123 , "
                                                       "\"s\" : \"abc\", "
                                                       "\"a\" : [ 1, 2, 3 ],"
                                                       "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                                       " } "
    ));
    EXPECT_EQ(LeptType::kLeptObject, LeptGetType(v));
    EXPECT_EQ(7, LeptGetObjectSize(v));
    EXPECT_STREQ("n", LeptGetObjectKey(v, 0));
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(LeptGetObjectValue(v, 0)));
    EXPECT_STREQ("f", LeptGetObjectKey(v, 1));
    EXPECT_EQ(LeptType::kLeptFalse, LeptGetType(LeptGetObjectValue(v, 1)));
    EXPECT_STREQ("t", LeptGetObjectKey(v, 2));
    EXPECT_EQ(LeptType::kLeptTrue, LeptGetType(LeptGetObjectValue(v, 2)));
    EXPECT_STREQ("i", LeptGetObjectKey(v, 3));
    EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(LeptGetObjectValue(v, 3)));
    EXPECT_DOUBLE_EQ(123.0, LeptGetNumber(LeptGetObjectValue(v, 3)));
    EXPECT_STREQ("s", LeptGetObjectKey(v, 4));
    EXPECT_EQ(LeptType::kLeptString, LeptGetType(LeptGetObjectValue(v, 4)));
    EXPECT_STREQ("abc", LeptGetString(LeptGetObjectValue(v, 4)));
    EXPECT_STREQ("a", LeptGetObjectKey(v, 5));
    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(LeptGetObjectValue(v, 5)));
    EXPECT_EQ(3, LeptGetArraySize(LeptGetObjectValue(v, 5)));
    for (auto i = 0; i < 3; ++i)
    {
        auto e = LeptGetArrayElement(LeptGetObjectValue(v, 5), i);
        EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(e));
        EXPECT_EQ(i + 1, LeptGetNumber(e));
    }
    EXPECT_STREQ("o", LeptGetObjectKey(v, 6));
    {
        auto o = LeptGetObjectValue(v, 6);
        EXPECT_EQ(LeptType::kLeptObject, LeptGetType(o));
        for (auto i = 0; i < 3; ++i)
        {
            auto ov = LeptGetObjectValue(o , i);
            EXPECT_TRUE('1' + i == LeptGetObjectKey(o , i)[0]);
            EXPECT_EQ(1 , LeptGetObjectKeyLength(o , i));
            EXPECT_EQ(LeptType::kLeptNumber , LeptGetType(ov));
            EXPECT_DOUBLE_EQ(i + 1.0 , LeptGetNumber(ov));
        }
    }
}

class TestParseWrong : public ::testing::TestWithParam<pair<LeptParseStatus, const char *>>
{
public:

    LeptJson v;

    void SetUp() override
    {
        LeptInit(v);
    }

    void TearDown() override
    {
        LeptFree(v);
    }
};

TEST_P(TestParseWrong, test_parse_wrong)
{
    auto it = GetParam();
    v.type_ = LeptType::kLeptFalse;
    EXPECT_EQ(it.first, LeptParse(v, it.second)) << "wrong test_case is: " << it.second;
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(v)) << "wrong test_case is: " << it.second;
}

INSTANTIATE_TEST_CASE_P(MyTestParseWrong, TestParseWrong, ::testing::Values(
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseRootNotSingular, "null x"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseRootNotSingular, "0123"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseRootNotSingular, "0x0"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseRootNotSingular, "0x123"),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseNumberTooBig, "1e309"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseNumberTooBig, "-1e309"),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissQuotationMark, "\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissQuotationMark, "\"abc"),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidStringEscape, "\"\\v\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidStringEscape, "\"\\'\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidStringEscape, "\"\\0\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidStringEscape, "\"\\x12\""),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidStringChar, "\"\x01\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidStringChar, "\"\x1F\""),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u0\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u01\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u012\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u/000\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\uG000\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u0/00\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u0G00\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u0/00\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u00G0\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u000/\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u000G\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeHex, "\"\\u 123\""),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate, "\"\\uD800\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate, "\"\\uDBFF\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate, "\"\\uD800\\\\\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate, "\"\\uD800\\uDBFF\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseInvalidUnicodeSurrogate, "\"\\uD800\\uE000\""),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrSquareBracket, "[1"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrSquareBracket, "[1}"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrSquareBracket, "[1 , 2"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrSquareBracket, "[[]"),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{1:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{true:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{false:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{null:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{[]:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{{}:1,"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissKey, "{\"a\":1,"),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissColon, "{\"a\"}"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissColon, "{\"a\",\"b\"}"),

    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrCurlyBracket, "{\"a\":1"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrCurlyBracket, "{\"a\":1]"),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrCurlyBracket, "{\"a\":1 \"b\""),
    pair<LeptParseStatus, const char *>(LeptParseStatus::kLeptParseMissCommaOrCurlyBracket, "{\"a\":{}")

));

int main()
{
    return RUN_ALL_TESTS();
}