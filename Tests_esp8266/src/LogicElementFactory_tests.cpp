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
    LogicElement *element = LogicElementFactory::Create(TvElementType::et_Undef);

    CHECK(element == NULL);
    delete element;
}

TEST(LogicElementFactoryTestsGroup, Create_Network) {
    LogicElement *element = LogicElementFactory::Create(TvElementType::et_Network);

    CHECK(element != NULL);
    delete element;
}
