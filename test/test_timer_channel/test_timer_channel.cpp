#include <unity.h>

#include "TimerChannel.h"

namespace {

const pool::TimerChannelConfig kConfig = {1000, 100, 8, 200, 300};
const pool::TimerChannelConfig kNoStartupConfig = {1000, 0, 0, 0, 300};

}  // namespace

void setUp(void) {}
void tearDown(void) {}

void test_idle_starts_in_starting_state(void) {
  pool::TimerChannel timer(kConfig);
  TEST_ASSERT_TRUE(timer.start(50));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::Starting),
                        static_cast<int>(timer.state()));
  TEST_ASSERT_TRUE(timer.loadShouldBeOn());
  TEST_ASSERT_EQUAL_UINT32(1000, timer.remainingMs(800));
}

void test_starting_transitions_to_running_after_animation(void) {
  pool::TimerChannel timer(kConfig);
  timer.start(0);
  TEST_ASSERT_FALSE(timer.update(999));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::Starting),
                        static_cast<int>(timer.state()));
  TEST_ASSERT_TRUE(timer.update(1000));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::Running),
                        static_cast<int>(timer.state()));
  TEST_ASSERT_EQUAL_UINT32(1000, timer.remainingMs(1000));
}

void test_running_finishes_after_duration(void) {
  pool::TimerChannel timer(kNoStartupConfig);
  timer.start(0);
  timer.update(0);
  TEST_ASSERT_TRUE(timer.update(1000));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::FinishedFeedback),
                        static_cast<int>(timer.state()));
  TEST_ASSERT_FALSE(timer.loadShouldBeOn());
}

void test_running_cancels(void) {
  pool::TimerChannel timer(kNoStartupConfig);
  timer.start(0);
  timer.update(0);
  TEST_ASSERT_TRUE(timer.cancel(200));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::CanceledFeedback),
                        static_cast<int>(timer.state()));
  TEST_ASSERT_FALSE(timer.loadShouldBeOn());
}

void test_canceled_feedback_returns_to_idle(void) {
  pool::TimerChannel timer(kNoStartupConfig);
  timer.start(0);
  timer.update(0);
  timer.cancel(100);
  TEST_ASSERT_FALSE(timer.update(399));
  TEST_ASSERT_TRUE(timer.update(400));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::Idle),
                        static_cast<int>(timer.state()));
}

void test_finished_feedback_returns_to_idle(void) {
  pool::TimerChannel timer(kNoStartupConfig);
  timer.start(0);
  timer.update(0);
  timer.update(1000);
  TEST_ASSERT_TRUE(timer.update(1300));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::Idle),
                        static_cast<int>(timer.state()));
}

void test_channels_are_independent(void) {
  pool::TimerChannel left(kNoStartupConfig);
  pool::TimerChannel right(kNoStartupConfig);
  left.start(0);
  left.update(0);
  right.start(20);
  right.update(20);
  left.cancel(100);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::CanceledFeedback),
                        static_cast<int>(left.state()));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::ChannelState::Running),
                        static_cast<int>(right.state()));
}

void test_remaining_time_and_millis_overflow_are_safe(void) {
  pool::TimerChannel timer(kNoStartupConfig);
  const uint32_t startedAt = 0xFFFFFF00UL;
  timer.start(startedAt);
  timer.update(startedAt);
  TEST_ASSERT_EQUAL_UINT32(1000, timer.remainingMs(startedAt));
  TEST_ASSERT_EQUAL_UINT32(500, timer.remainingMs(startedAt + 500));
  TEST_ASSERT_EQUAL_UINT32(0, timer.remainingMs(startedAt + 1000));
  timer.update(startedAt + 1000);
  TEST_ASSERT_EQUAL_UINT32(0, timer.remainingMs(startedAt + 1001));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_idle_starts_in_starting_state);
  RUN_TEST(test_starting_transitions_to_running_after_animation);
  RUN_TEST(test_running_finishes_after_duration);
  RUN_TEST(test_running_cancels);
  RUN_TEST(test_canceled_feedback_returns_to_idle);
  RUN_TEST(test_finished_feedback_returns_to_idle);
  RUN_TEST(test_channels_are_independent);
  RUN_TEST(test_remaining_time_and_millis_overflow_are_safe);
  return UNITY_END();
}
