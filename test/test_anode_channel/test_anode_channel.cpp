#include <unity.h>

#include "AnodeChannel.h"

namespace {

const poolanode::AnodeChannelConfig kConfig = {1000, 100, 8, 200, 300};
const poolanode::AnodeChannelConfig kNoStartupConfig = {1000, 0, 0, 0, 300};

}  // namespace

void setUp(void) {}
void tearDown(void) {}

void test_idle_transitions_to_starting(void) {
  poolanode::AnodeChannel channel(kConfig);
  TEST_ASSERT_TRUE(channel.start(50));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(poolanode::AnodeState::Starting),
                        static_cast<int>(channel.state()));
  TEST_ASSERT_TRUE(channel.outputShouldBeEnabled());
}

void test_starting_transitions_to_running_after_startup_animation(void) {
  poolanode::AnodeChannel channel(kConfig);
  channel.start(0);
  TEST_ASSERT_FALSE(channel.update(999));
  TEST_ASSERT_TRUE(channel.update(1000));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(poolanode::AnodeState::Running),
                        static_cast<int>(channel.state()));
  TEST_ASSERT_EQUAL_UINT32(1000, channel.remainingRuntimeMs(1000));
}

void test_running_transitions_to_finished_feedback_after_runtime(void) {
  poolanode::AnodeChannel channel(kNoStartupConfig);
  channel.start(0);
  channel.update(0);
  TEST_ASSERT_TRUE(channel.update(1000));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::AnodeState::FinishedFeedback),
      static_cast<int>(channel.state()));
  TEST_ASSERT_FALSE(channel.outputShouldBeEnabled());
}

void test_running_transitions_to_canceled_feedback(void) {
  poolanode::AnodeChannel channel(kNoStartupConfig);
  channel.start(0);
  channel.update(0);
  TEST_ASSERT_TRUE(channel.cancel(200));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::AnodeState::CanceledFeedback),
      static_cast<int>(channel.state()));
  TEST_ASSERT_FALSE(channel.outputShouldBeEnabled());
}

void test_canceled_feedback_returns_to_idle(void) {
  poolanode::AnodeChannel channel(kNoStartupConfig);
  channel.start(0);
  channel.update(0);
  channel.cancel(100);
  TEST_ASSERT_FALSE(channel.update(399));
  TEST_ASSERT_TRUE(channel.update(400));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(poolanode::AnodeState::Idle),
                        static_cast<int>(channel.state()));
}

void test_finished_feedback_returns_to_idle(void) {
  poolanode::AnodeChannel channel(kNoStartupConfig);
  channel.start(0);
  channel.update(0);
  channel.update(1000);
  TEST_ASSERT_TRUE(channel.update(1300));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(poolanode::AnodeState::Idle),
                        static_cast<int>(channel.state()));
}

void test_swimming_pool_and_whirlpool_channels_are_independent(void) {
  poolanode::AnodeChannel swimmingPool(kNoStartupConfig);
  poolanode::AnodeChannel whirlpool(kNoStartupConfig);
  swimmingPool.start(0);
  swimmingPool.update(0);
  whirlpool.start(20);
  whirlpool.update(20);
  swimmingPool.cancel(100);
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::AnodeState::CanceledFeedback),
      static_cast<int>(swimmingPool.state()));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(poolanode::AnodeState::Running),
                        static_cast<int>(whirlpool.state()));
}

void test_remaining_runtime_is_correct_and_overflow_safe(void) {
  poolanode::AnodeChannel channel(kNoStartupConfig);
  const uint32_t startedAt = 0xFFFFFF00UL;
  channel.start(startedAt);
  channel.update(startedAt);
  TEST_ASSERT_EQUAL_UINT32(1000, channel.remainingRuntimeMs(startedAt));
  TEST_ASSERT_EQUAL_UINT32(500, channel.remainingRuntimeMs(startedAt + 500));
  TEST_ASSERT_EQUAL_UINT32(0, channel.remainingRuntimeMs(startedAt + 1000));
  channel.update(startedAt + 1000);
  TEST_ASSERT_EQUAL_UINT32(0, channel.remainingRuntimeMs(startedAt + 1001));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_idle_transitions_to_starting);
  RUN_TEST(test_starting_transitions_to_running_after_startup_animation);
  RUN_TEST(test_running_transitions_to_finished_feedback_after_runtime);
  RUN_TEST(test_running_transitions_to_canceled_feedback);
  RUN_TEST(test_canceled_feedback_returns_to_idle);
  RUN_TEST(test_finished_feedback_returns_to_idle);
  RUN_TEST(test_swimming_pool_and_whirlpool_channels_are_independent);
  RUN_TEST(test_remaining_runtime_is_correct_and_overflow_safe);
  return UNITY_END();
}
