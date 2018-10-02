
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
    }

    LeptJson null_;
    LeptJson expect_value_;
    LeptJson true_;
    LeptJson false_;
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

//TEST_F(TestArray, test_array)
//{
//    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, "[ ]"));
//    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(v));
//    EXPECT_EQ(0, LeptGetArraySize(v));
//    LeptFree(v);
//
//    LeptInit(v);
//    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, "[ null , false ,true , 123 , \"abc\" ]"));
//    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(v));
//    EXPECT_EQ(5, LeptGetArraySize(v));
//    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(LeptGetArrayElement(v, 0)));
//    EXPECT_EQ(LeptType::kLeptFalse, LeptGetType(LeptGetArrayElement(v, 1)));
//    EXPECT_EQ(LeptType::kLeptTrue, LeptGetType(LeptGetArrayElement(v, 2)));
//    EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(LeptGetArrayElement(v, 3)));
//    EXPECT_EQ(LeptType::kLeptString, LeptGetType(LeptGetArrayElement(v, 4)));
//    EXPECT_DOUBLE_EQ(123.0, LeptGetNumber(LeptGetArrayElement(v, 3)));
//    EXPECT_STREQ("abc", LeptGetString(LeptGetArrayElement(v, 4)));
//    LeptFree(v);
//
//    LeptInit(v);
//    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, "[ [ ] , [ 0 ] , [ 0  , 1 ] , [ 0 , 1, 2 ] ]"));
//    EXPECT_EQ(LeptType::kLeptArray, LeptGetType(v));
//    EXPECT_EQ(4, LeptGetArraySize(v));
//    for (size_t i = 0; i < 4; ++i)
//    {
//        auto a = LeptGetArrayElement(v, i);
//        EXPECT_EQ(LeptType::kLeptArray, LeptGetType(a));
//        EXPECT_EQ(i, LeptGetArraySize(a));
//        for (auto j = 0; j < i; ++j)
//        {
//            auto e = LeptGetArrayElement(a, j);
//            EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(e));
//            EXPECT_EQ((double) j, LeptGetNumber(e));
//        }
//    }
//}

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
    auto o = LeptGetObjectValue(v, 6);
    EXPECT_EQ(LeptType::kLeptObject, LeptGetType(o));
    for (auto i = 0; i < 3; ++i)
    {
        auto ov = LeptGetObjectValue(o, i);
        EXPECT_TRUE('1' + i == LeptGetObjectKey(o, i)[0]);
        EXPECT_EQ(1, LeptGetObjectKeyLength(o, i));
        EXPECT_EQ(LeptType::kLeptNumber, LeptGetType(ov));
        EXPECT_DOUBLE_EQ(i + 1.0, LeptGetNumber(ov));
    }
    LeptFree(o);
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

class TestStringify : public ::testing::TestWithParam<const char *>
{
public:

    LeptJson v;
    char *json2;

    void SetUp() override
    {
        LeptInit(v);
        json2 = nullptr;
    }

    void TearDown() override
    {
        LeptFree(v);
        free(json2);
    }
};

TEST_P(TestStringify, test_stringify)
{
    auto it = GetParam();
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(v, it));
    size_t len;
    json2 = LeptStringify(v, len);
    EXPECT_STREQ(json2, it);
}

INSTANTIATE_TEST_CASE_P(MyTestStringify, TestStringify, ::testing::Values(
    "null",
    "false",
    "true",
    "0",
    "-0",
    "1",
    "-1",
    "1.5",
    "-1.5",
    "3.25",
    "1e+20",
    "1.234e+20",
    "1.234e-20",
    "-1.234e+20",
    "1.0000000000000002",
    "4.9406564584124654e-324",
    "-4.9406564584124654e-324",
    "2.2250738585072009e-308",
    "-2.2250738585072009e-308",
    "2.2250738585072014e-308",
    "-2.2250738585072014e-308",
    "1.7976931348623157e+308",
    "-1.7976931348623157e+308",
    "\"\"",
    "\"Hello\"",
    "\"Hello\\nWorld\"",
    "\"\\\" \\\\ / \\b \\f \\n \\r \\t\"",
    "\"Hello\\u0000World\"",
    "[]",
    "[null,false,true,123,\"abc\",[1,2,3]]",
    "{}",
    "{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}"
));

class TestEqual : public ::testing::TestWithParam<tuple<const char *, const char *, int>>
{
public:

    LeptJson lhs, rhs;

    void SetUp() override
    {
        LeptInit(lhs);
        LeptInit(rhs);
    }

    void TearDown() override
    {
        LeptFree(lhs);
        LeptFree(rhs);
    }
};

TEST_P(TestEqual, test_equal)
{
    auto it = GetParam();
    auto json1 = std::get<0>(it);
    auto json2 = std::get<1>(it);
    auto eql = std::get<2>(it);
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(lhs, json1));
    EXPECT_EQ(LeptParseStatus::kLeptParseOk, LeptParse(rhs, json2));
    EXPECT_EQ(eql, (lhs == rhs));
}

INSTANTIATE_TEST_CASE_P(MyTestEqual, TestEqual, ::testing::Values(
    make_tuple("true", "true", 1),
    make_tuple("true", "false", 0),
    make_tuple("false", "false", 1),
    make_tuple("null", "null", 1),
    make_tuple("null", "0", 0),
    make_tuple("123", "123", 1),
    make_tuple("123", "456", 0),
    make_tuple("\"abc\"", "\"abc\"", 1),
    make_tuple("\"abc\"", "\"abcd\"", 0),
    make_tuple("[]", "[]", 1),
    make_tuple("[]", "null", 0),
    make_tuple("[1,2,3]", "[1,2,3]", 1),
    make_tuple("[1,2,3]", "[1,2,3,4]", 0),
    make_tuple("[[]]", "[[]]", 1),
    make_tuple("{}", "{}", 1),
    make_tuple("{}", "null", 0),
    make_tuple("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1),
    make_tuple("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1),
    make_tuple("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0),
    make_tuple("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0),
    make_tuple("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1),
    make_tuple("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0)
));

class TestCopy : public ::testing::Test
{
public:
    LeptJson lhs;

    void SetUp() override
    {
        LeptInit(lhs);
    }

    void TearDown() override
    {
        LeptFree(lhs);
    }
};

TEST_F(TestCopy, test_copy)
{
    LeptParse(lhs, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    LeptJson rhs(lhs);
    EXPECT_TRUE(lhs == rhs);
    LeptFree(rhs);
}

//Just use for TEST_F
class TestForF : public ::testing::Test {};

//TEST_F(TestForF, test_move)
//{
//    LeptJson v1;
//    LeptInit(v1);
//    LeptParse(v1, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
//
//    LeptJson v2(v1);
//    LeptJson v3(std::move(v2));
//
//    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(v2));
//    EXPECT_TRUE(v1 == v3);
//
//    LeptFree(v1);
//    LeptFree(v2);
//    LeptFree(v3);
//}

TEST_F(TestForF, test_swap)
{
    LeptJson v1, v2;
    LeptInit(v1);
    LeptInit(v2);
    LeptSetString(v1, "Hello", 5);
    LeptSetString(v2, "World!", 6);
    v1.Swap(v2);
    EXPECT_STREQ("World!", LeptGetString(v1));
    EXPECT_STREQ("Hello", LeptGetString(v2));

    LeptFree(v1);
    LeptFree(v2);
}

TEST_F(TestForF, test_access_null)
{
    LeptJson v;
    LeptSetString(v, "a", 1);
    LeptSetNull(v);
    EXPECT_EQ(LeptType::kLeptNull, LeptGetType(v));
    LeptFree(v);
}

TEST_F(TestForF, test_access_boolean)
{
    LeptJson v;
    LeptSetString(v, "a", 1);
    LeptSetBoolean(v, 1);
    EXPECT_TRUE(LeptGetBoolean(v));
    LeptSetBoolean(v , 0);
    EXPECT_FALSE(LeptGetBoolean(v));
    LeptFree(v);
}

TEST_F(TestForF, test_access_number)
{
    LeptJson v;
    LeptSetString(v, "A", 1);
    LeptSetNumber(v, 1234.5);
    EXPECT_DOUBLE_EQ(1234.5, LeptGetNumber(v));
    EXPECT_DOUBLE_EQ(1234.5, LeptGetNumber(v));
    LeptFree(v);
}

TEST_F(TestForF, test_access_string)
{
    LeptJson v;
    LeptSetString(v, "", 0);
    EXPECT_STREQ("", LeptGetString(v));
    LeptSetString(v, "Hello", 5);
    EXPECT_STREQ("Hello", LeptGetString(v));
    LeptFree(v);
}
//
//TEST_F(TestForF, test_access_array)
//{
//    LeptJson a, e;
//
//    for (int j = 0; j <= 5; j += 5)
//    {
//        LeptSetArray(a, j);
//        EXPECT_EQ(0, LeptGetArraySize(a));
//        EXPECT_EQ(j, LeptGetArrayCapacity(a));
//        for (int i = 0; i < 10; ++i)
//        {
//            LeptInit(e);
//            LeptSetNumber(e, i);
//            LeptPushbackArrayElement(a) = std::move(e);
//            LeptFree(e);
//        }
//
//        EXPECT_EQ(10, LeptGetArraySize(a));
//        for (int i = 0; i < 10; ++i)
//            EXPECT_DOUBLE_EQ(static_cast<double>(i), LeptGetNumber(LeptGetArrayElement(a, i)));
//    }
//
//    LeptPopbackArrayElement(a);
//    EXPECT_EQ(9, LeptGetArraySize(a));
//    for (int i = 0; i < 9; i++)
//        EXPECT_DOUBLE_EQ((double) i, LeptGetNumber(LeptGetArrayElement(a, i)));
//
//    //erase
//    LeptEraseArrayElement(a, 4, 0);
//    EXPECT_EQ(9, LeptGetArraySize(a)); //FIXME: Not 8?
//    for (int i = 0; i < 9; i++)
//        EXPECT_DOUBLE_EQ(double(i), LeptGetNumber(LeptGetArrayElement(a, i)));
//
//    LeptEraseArrayElement(a, 8, 1);
//    EXPECT_EQ(8, LeptGetArraySize(a));
//    for (int i = 0; i < 8; ++i)
//        EXPECT_DOUBLE_EQ(double(i), LeptGetNumber(LeptGetArrayElement(a, i)));
//
//    LeptEraseArrayElement(a, 0, 2);
//    EXPECT_EQ(6, LeptGetArraySize(a));
//    for (int i = 0; i < 6; ++i)
//        EXPECT_DOUBLE_EQ(double(i
//                             +2), LeptGetNumber(LeptGetArrayElement(a, i)));
//
//    for(int i = 0; i  < 2; ++i)
//    {
//        LeptInit(e);
//        LeptSetNumber(e , i);
//        LeptInsertArrayElement(a , i) = std::move(e);
//        LeptFree(e);
//    }
//
//    EXPECT_EQ(8 , LeptGetArraySize(a));
//    for (int i = 0; i < 8; ++i)
//        EXPECT_DOUBLE_EQ(double(i), LeptGetNumber(LeptGetArrayElement(a, i)));
//
//    EXPECT_TRUE(LeptGetArrayCapacity(a) > 8);
//    LeptShrinkArray(a);
//    EXPECT_EQ(8 , LeptGetArraySize(a));
//    EXPECT_EQ(8 , LeptGetArrayCapacity(a));
//    for(int i = 0;  i< 8 ; ++i)
//        EXPECT_DOUBLE_EQ(double(i) , LeptGetNumber(LeptGetArrayElement(a , i)));
//
//    LeptSetString(e , "Hello" , 5);
//    LeptPushbackArrayElement(a) = std::move(e);
//    LeptFree(e);
//
//    auto i = LeptGetArrayCapacity(a);
//    LeptClearArray(a);
//    EXPECT_EQ(0 , LeptGetArraySize(a));
//    EXPECT_EQ(i , LeptGetArrayCapacity(a));
//    LeptShrinkArray(a);
//    EXPECT_EQ(0 , LeptGetArrayCapacity(a));
//}

//TEST_F(TestForF , test_access_object)
//{
//    LeptJson o , v;
//
//    for(int j = 0 ; j <= 5; j += 5)
//    {
//        LeptSetObject(o , j);
//        EXPECT_EQ(0 , LeptGetObjectSize(o));
//        EXPECT_EQ(j , LeptGetObjectCapacity(o));
//        for(int i = 0 ; i < 10; ++i)
//        {
//            char key[2] = "a";
//            key[0] += i;
//            LeptInit(v);
//            LeptSetNumber(v , i);
//            LeptSetObjectValue(o , key , 1) = std::move(v);
//            LeptFree(v);
//        }
//        EXPECT_EQ(10 ,  LeptGetObjectSize(o));
//        for(int i = 0 ; i < 10; ++i)
//        {
//            char key[] = "a";
//            key[0] += i;
//            auto index = LeptFindObjectIndex(o , key , 1);
//            EXPECT_TRUE(index != kLeptKeyNotExist);
//            EXPECT_DOUBLE_EQ(double(i) ,  LeptGetNumber(LeptGetObjectValue(o , index)));
//        }
//        auto index = LeptFindObjectIndex(o , "j" ,1);
//        EXPECT_TRUE(index != kLeptKeyNotExist);
//        LeptRemoveObjectValue(o , index);
//        index = LeptFindObjectIndex(o , "j" , 1);
//        EXPECT_TRUE(index == kLeptKeyNotExist);
//        EXPECT_EQ(9  , LeptGetObjectSize(o));
//
//        index = LeptFindObjectIndex(o , "a" , 1);
//        EXPECT_TRUE(index != kLeptKeyNotExist);
//        LeptRemoveObjectValue(o , index);
//        index = LeptFindObjectIndex(o , "a" , 1);
//        EXPECT_TRUE(index == kLeptKeyNotExist);
//        EXPECT_EQ(8  , LeptGetObjectSize(o));
//
//        EXPECT_TRUE(LeptGetObjectCapacity(o) > 8);
//        LeptShrinkArray(o);
//        EXPECT_EQ(8 , LeptGetObjectSize(o));
//        EXPECT_EQ(8 , LeptGetObjectCapacity(o));
//        for(int i = 0 ; i < 8; ++i)
//        {
//            char key[] = "a";
//            key[0] += i + 1;
//            EXPECT_DOUBLE_EQ(double(i) , LeptGetNumber(LeptGetObjectValue(o , LeptFindObjectIndex(o , key , 1))));
//        }
//
//        LeptSetString(v , "Hello" , 5);
//        LeptSetObjectValue(o , "World" , 5) = std::move(v);
//        LeptFree(v);
//
//        auto pv = LeptFindObjectValue(o , "World" , 5);
//        EXPECT_STREQ("Hello" , LeptGetString(pv));
//
//        auto i  = LeptGetObjectCapacity(o);
//        LeptClearObejct(o);
//        EXPECT_EQ(0 , LeptGetObjectSize(o));
//        EXPECT_EQ(i , LeptGetObjectCapacity(o));
//        LeptShrinkArray(o);
//        EXPECT_EQ(0 , LeptGetObjectCapacity(o));
//    }
//}

int main()
{
    return RUN_ALL_TESTS();
}