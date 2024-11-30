#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

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
    CHECK_TRUE(InputElement::TryToCast(&inputNC) == &inputNC);

    InputNO inputNO;
    CHECK_TRUE(InputElement::TryToCast(&inputNO) == &inputNO);

    ComparatorEq comparatorEq;
    CHECK_TRUE(InputElement::TryToCast(&comparatorEq) == &comparatorEq);

    ComparatorGE comparatorGE;
    CHECK_TRUE(InputElement::TryToCast(&comparatorGE) == &comparatorGE);

    ComparatorGr comparatorGr;
    CHECK_TRUE(InputElement::TryToCast(&comparatorGr) == &comparatorGr);

    ComparatorLE comparatorLE;
    CHECK_TRUE(InputElement::TryToCast(&comparatorLE) == &comparatorLE);

    ComparatorLs comparatorLs;
    CHECK_TRUE(InputElement::TryToCast(&comparatorLs) == &comparatorLs);

    TimerMSecs timerMSecs;
    CHECK_TRUE(InputElement::TryToCast(&timerMSecs) == NULL);

    TimerSecs timerSecs;
    CHECK_TRUE(InputElement::TryToCast(&timerSecs) == NULL);

    DirectOutput directOutput;
    CHECK_TRUE(InputElement::TryToCast(&directOutput) == &directOutput);

    SetOutput setOutput;
    CHECK_TRUE(InputElement::TryToCast(&setOutput) == &setOutput);

    ResetOutput resetOutput;
    CHECK_TRUE(InputElement::TryToCast(&resetOutput) == &resetOutput);

    IncOutput incOutput;
    CHECK_TRUE(InputElement::TryToCast(&incOutput) == &incOutput);

    DecOutput decOutput;
    CHECK_TRUE(InputElement::TryToCast(&decOutput) == &decOutput);
}
