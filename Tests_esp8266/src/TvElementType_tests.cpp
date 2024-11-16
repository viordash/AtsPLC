#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Serializer/TypeValueElement.h"

TEST_GROUP(TvElementTypeTestsGroup){ //
                                     TEST_SETUP(){}

                                     TEST_TEARDOWN(){}
};

TEST(TvElementTypeTestsGroup, IsInputElement) {
    CHECK_FALSE(IsInputElement(TvElementType::et_Undef));
    CHECK_TRUE(IsInputElement(TvElementType::et_InputNC));
    CHECK_TRUE(IsInputElement(TvElementType::et_InputNO));
    CHECK_TRUE(IsInputElement(TvElementType::et_TimerSecs));
    CHECK_TRUE(IsInputElement(TvElementType::et_TimerMSecs));
    CHECK_TRUE(IsInputElement(TvElementType::et_ComparatorEq));
    CHECK_TRUE(IsInputElement(TvElementType::et_ComparatorGE));
    CHECK_TRUE(IsInputElement(TvElementType::et_ComparatorGr));
    CHECK_TRUE(IsInputElement(TvElementType::et_ComparatorLE));
    CHECK_TRUE(IsInputElement(TvElementType::et_ComparatorLs));
    CHECK_FALSE(IsInputElement(TvElementType::et_DirectOutput));
    CHECK_FALSE(IsInputElement(TvElementType::et_SetOutput));
    CHECK_FALSE(IsInputElement(TvElementType::et_ResetOutput));
    CHECK_FALSE(IsInputElement(TvElementType::et_IncOutput));
    CHECK_FALSE(IsInputElement(TvElementType::et_DecOutput));
}

TEST(TvElementTypeTestsGroup, IsOutputElement) {
    CHECK_FALSE(IsOutputElement(TvElementType::et_Undef));
    CHECK_FALSE(IsOutputElement(TvElementType::et_InputNC));
    CHECK_FALSE(IsOutputElement(TvElementType::et_InputNO));
    CHECK_FALSE(IsOutputElement(TvElementType::et_TimerSecs));
    CHECK_FALSE(IsOutputElement(TvElementType::et_TimerMSecs));
    CHECK_FALSE(IsOutputElement(TvElementType::et_ComparatorEq));
    CHECK_FALSE(IsOutputElement(TvElementType::et_ComparatorGE));
    CHECK_FALSE(IsOutputElement(TvElementType::et_ComparatorGr));
    CHECK_FALSE(IsOutputElement(TvElementType::et_ComparatorLE));
    CHECK_FALSE(IsOutputElement(TvElementType::et_ComparatorLs));
    CHECK_TRUE(IsOutputElement(TvElementType::et_DirectOutput));
    CHECK_TRUE(IsOutputElement(TvElementType::et_SetOutput));
    CHECK_TRUE(IsOutputElement(TvElementType::et_ResetOutput));
    CHECK_TRUE(IsOutputElement(TvElementType::et_IncOutput));
    CHECK_TRUE(IsOutputElement(TvElementType::et_DecOutput));
}

TEST(TvElementTypeTestsGroup, IsIndicatorElement) {
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_Undef));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_InputNC));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_InputNO));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_TimerSecs));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_TimerMSecs));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_ComparatorEq));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_ComparatorGE));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_ComparatorGr));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_ComparatorLE));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_ComparatorLs));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_DirectOutput));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_SetOutput));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_ResetOutput));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_IncOutput));
    CHECK_FALSE(IsIndicatorElement(TvElementType::et_DecOutput));
    CHECK_TRUE(IsIndicatorElement(TvElementType::et_Indicator));
}
