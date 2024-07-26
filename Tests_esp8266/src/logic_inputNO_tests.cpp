#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/InputNO.cpp"
#include "main/LogicProgram/InputNO.h"

TEST_GROUP(LogicInputNOGroup){ //
                               TEST_SETUP(){}

                               TEST_TEARDOWN(){}
};

class TestableInputNO : public InputNO {
  public:
    TestableInputNO(const MapIO io_adr, const Point &location) : InputNO(io_adr, location) {
    }
    virtual ~TestableInputNO() {
    }

    const char *GetLabel() {
        return label;
    }
};

TEST(LogicInputNOGroup, GetLabel_DI) {
    TestableInputNO testable(MapIO::DI, { 0, 0 });
    STRCMP_EQUAL("DI", testable.GetLabel());
}

TEST(LogicInputNOGroup, GetLabel_AI) {
    TestableInputNO testable(MapIO::AI, { 0, 0 });
    STRCMP_EQUAL("AI", testable.GetLabel());
}

TEST(LogicInputNOGroup, GetLabel_V1) {
    TestableInputNO testable(MapIO::V1, { 0, 0 });
    STRCMP_EQUAL("V1", testable.GetLabel());
}