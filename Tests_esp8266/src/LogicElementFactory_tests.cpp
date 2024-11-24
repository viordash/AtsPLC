#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Serializer/LogicElementFactory.cpp"

TEST_GROUP(LogicElementFactoryTestsGroup){ //
                                           TEST_SETUP(){}

                                           TEST_TEARDOWN(){}
};

TEST(LogicElementFactoryTestsGroup, Create_Undef_return_null) {
    auto element = LogicElementFactory::Create(TvElementType::et_Undef);
    CHECK(element == NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_InputNC) {
    auto element = LogicElementFactory::Create(TvElementType::et_InputNC);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_InputNO) {
    auto element = LogicElementFactory::Create(TvElementType::et_InputNO);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_TimerSecs) {
    auto element = LogicElementFactory::Create(TvElementType::et_TimerSecs);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_TimerMSecs) {
    auto element = LogicElementFactory::Create(TvElementType::et_TimerMSecs);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_ComparatorEq) {
    auto element = LogicElementFactory::Create(TvElementType::et_ComparatorEq);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_ComparatorGE) {
    auto element = LogicElementFactory::Create(TvElementType::et_ComparatorGE);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_ComparatorGr) {
    auto element = LogicElementFactory::Create(TvElementType::et_ComparatorGr);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_ComparatorLE) {
    auto element = LogicElementFactory::Create(TvElementType::et_ComparatorLE);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_ComparatorLs) {
    auto element = LogicElementFactory::Create(TvElementType::et_ComparatorLs);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_DirectOutput) {
    auto element = LogicElementFactory::Create(TvElementType::et_DirectOutput);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_SetOutput) {
    auto element = LogicElementFactory::Create(TvElementType::et_SetOutput);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_ResetOutput) {
    auto element = LogicElementFactory::Create(TvElementType::et_ResetOutput);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_IncOutput) {
    auto element = LogicElementFactory::Create(TvElementType::et_IncOutput);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_DecOutput) {
    auto element = LogicElementFactory::Create(TvElementType::et_DecOutput);
    CHECK(element != NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_Wire) {
    auto element = LogicElementFactory::Create(TvElementType::et_Wire);
    CHECK(element != NULL);
    delete element;
}
