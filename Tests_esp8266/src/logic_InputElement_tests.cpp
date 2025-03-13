#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "LogicProgram/Bindings/WiFiBinding.h"
#include "main/LogicProgram/Inputs/CommonInput.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"
#include "main/LogicProgram/Inputs/TimerMSecs.h"
#include "main/LogicProgram/Inputs/TimerSecs.h"
#include "main/LogicProgram/Outputs/DecOutput.h"
#include "main/LogicProgram/Outputs/DirectOutput.h"
#include "main/LogicProgram/Outputs/IncOutput.h"
#include "main/LogicProgram/Outputs/ResetOutput.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

TEST_GROUP(LogicInputElementTestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

TEST(LogicInputElementTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK(InputElement::TryToCast(&inputNC) == &inputNC);

    InputNO inputNO;
    CHECK(InputElement::TryToCast(&inputNO) == &inputNO);

    ComparatorEq comparatorEq;
    CHECK(InputElement::TryToCast(&comparatorEq) == &comparatorEq);

    ComparatorGE comparatorGE;
    CHECK(InputElement::TryToCast(&comparatorGE) == &comparatorGE);

    ComparatorGr comparatorGr;
    CHECK(InputElement::TryToCast(&comparatorGr) == &comparatorGr);

    ComparatorLE comparatorLE;
    CHECK(InputElement::TryToCast(&comparatorLE) == &comparatorLE);

    ComparatorLs comparatorLs;
    CHECK(InputElement::TryToCast(&comparatorLs) == &comparatorLs);

    TimerMSecs timerMSecs;
    CHECK(InputElement::TryToCast(&timerMSecs) == NULL);

    TimerSecs timerSecs;
    CHECK(InputElement::TryToCast(&timerSecs) == NULL);

    DirectOutput directOutput;
    CHECK(InputElement::TryToCast(&directOutput) == NULL);

    SetOutput setOutput;
    CHECK(InputElement::TryToCast(&setOutput) == NULL);

    ResetOutput resetOutput;
    CHECK(InputElement::TryToCast(&resetOutput) == NULL);

    IncOutput incOutput;
    CHECK(InputElement::TryToCast(&incOutput) == NULL);

    DecOutput decOutput;
    CHECK(InputElement::TryToCast(&decOutput) == NULL);

    WiFiBinding wiFiBinding;
    CHECK(InputElement::TryToCast(&wiFiBinding) == &wiFiBinding);
}
