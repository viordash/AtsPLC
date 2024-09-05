// #include "CppUTest/TestHarness.h"
// #include "CppUTestExt/MockSupport.h"

// #include <errno.h>
// #include <stdarg.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <sys/stat.h>
// #include <unistd.h>

// #include "main/LogicProgram/Ladder.cpp"
// #include "main/LogicProgram/LogicProgram.h"

// static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

// TEST_GROUP(LogicLadderTestsGroup){ //
//                                    TEST_SETUP(){ mock().disable();
// memset(frame_buffer, 0, sizeof(frame_buffer));
// }

// TEST_TEARDOWN() {
//     mock().enable();
// }
// }
// ;

// namespace {
//     static const Bitmap bitmap = { //
//         { 16,                      // width
//           16 },                    // height
//         { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
//           0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
//           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
//     };

//     class MonitorLogicElement {
//       public:
//         bool DoAction_called = false;
//         bool DoAction_result = false;
//         bool Render_called = false;
//         bool Render_result = true;

//         bool DoAction() {
//             DoAction_called = true;
//             return DoAction_result;
//         }

//         bool Render() {
//             Render_called = true;
//             return Render_result;
//         }
//     };

//     class TestableInputNC : public InputNC, public MonitorLogicElement {
//       public:
//         TestableInputNC() : InputNC() {
//         }

//         bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
//             (void)prev_changed;
//             (void)prev_elem_state;
//             return MonitorLogicElement::DoAction();
//         }

//         bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
//             (void)fb;
//             (void)prev_elem_state;
//             (void)start_point;
//             return MonitorLogicElement::Render();
//         }
//     };

//     class TestableComparatorEq : public ComparatorEq, public MonitorLogicElement {
//       public:
//         TestableComparatorEq() : ComparatorEq() {
//         }
//         bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
//             (void)prev_changed;
//             (void)prev_elem_state;
//             return MonitorLogicElement::DoAction();
//         }

//         bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
//             (void)fb;
//             (void)prev_elem_state;
//             (void)start_point;
//             return MonitorLogicElement::Render();
//         }
//     };

//     class TestableTimerMSecs : public TimerMSecs, public MonitorLogicElement {
//       public:
//         explicit TestableTimerMSecs() : TimerMSecs() {
//         }

//         bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
//             (void)prev_changed;
//             (void)prev_elem_state;
//             return MonitorLogicElement::DoAction();
//         }

//         bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
//             (void)fb;
//             (void)prev_elem_state;
//             (void)start_point;
//             return MonitorLogicElement::Render();
//         }
//     };

//     class TestableDirectOutput : public DirectOutput, public MonitorLogicElement {
//       public:
//         explicit TestableDirectOutput() : DirectOutput() {
//         }
//         bool DoAction(bool prev_changed, LogicItemState prev_elem_state) override {
//             (void)prev_changed;
//             (void)prev_elem_state;
//             return MonitorLogicElement::DoAction();
//         }

//         bool Render(uint8_t *fb, LogicItemState prev_elem_state, Point *start_point) override {
//             (void)fb;
//             (void)prev_elem_state;
//             (void)start_point;
//             return MonitorLogicElement::Render();
//         }
//     };
// } // namespace

// TEST(LogicLadderTestsGroup, Store) {
//     Ladder testable;
//     auto network = new Network();
//     testable.Append(network);

//     network->ChangeState(LogicItemState::lisActive);
//     auto input = new TestableInputNC;
//     input->SetIoAdr(MapIO::DI);
//     network->Append(input);
//     auto comparator = new TestableComparatorEq();
//     comparator->SetReference(5);
//     comparator->SetIoAdr(MapIO::AI);
//     network->Append(comparator);
//     auto timer = new TestableTimerMSecs;
//     timer->SetTime(12345);
//     network->Append(timer);
//     auto output = new TestableDirectOutput;
//     output->SetIoAdr(MapIO::O1);
//     network->Append(output);

//     testable.Store();
// }
