
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
        pair<double, char*>( 0.0, "0" )
//        {0.0, "-0"},
//        {0.0 , "-0.0"},
//        {1.0 , "1"},
//        {-1.0 , "-1"},
//        {1.5 ,"1.5"},
//        {-1.5 ,"-1.5"},
//        {3.1416 , "3.1416"},
//        {1E10 , "1E10"},
//        {1e10 , "1e10"},
//        {1E+10 , "1E+10"},
//        {1E-10 , "1E-10"},
//        {-1E10 , "-1E10"},
//        {-1e10 , "-1e10"},
//        {-1E+10, "-1E+10"},
//        {-1E-10 , "-1E-10"},
//        {1.234E+10 , "1.234E+10"},
//        {1.234E-10 , "1.234E-10"},
//        {0.0 , "1e-10000"},
//        {1.0000000000000002, "1.0000000000000002"},
//        {4.9406564584124654e-324, "4.9406564584124654e-324"},
//        {-4.9406564584124654e-324, "-4.9406564584124654e-324"},
//        {2.2250738585072009e-308, "2.2250738585072009e-308"},
//        {-2.2250738585072009e-308, "-2.2250738585072009e-308"},
//        {2.2250738585072014e-308, "2.2250738585072014e-308"},
//        {-2.2250738585072014e-308, "-2.2250738585072014e-308"},
//        {1.7976931348623157e+308, "1.7976931348623157e+308"},
//        {-1.7976931348623157e+308, "-1.7976931348623157e+308"}
));

class TestWrongNumber : public ::testing::TestWithParam<const char*>
{
};

TEST_P(TestWrongNumber , test_wrong_number)
{
    auto it = GetParam();
    LeptJson v;
    v.type_ = LeptType::kLeptFalse;
    EXPECT_EQ(LeptParseStatus::kLeptParseInvalidValue , LeptParse(v , it)) << "wrong number is: " << it;
    EXPECT_EQ(LeptType::kLeptNull , LeptGetType(v)) <<  "wrong number is: " << it;
}

INSTANTIATE_TEST_CASE_P(MyTestWrongNumber , TestWrongNumber , ::testing::Values
(
    "+0",
    "+1",
    ".123",
    "1.",
    "INF" ,
    "inf",
    "NAN",
    "nan"
    ));

int main()
{
    return RUN_ALL_TESTS();
}