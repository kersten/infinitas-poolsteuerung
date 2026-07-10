#include <unity.h>

#include "ButtonDebouncer.h"

void setUp(void) {}
void tearDown(void) {}

void test_active_low_button_press_is_detected_once(void) {
  poolanode::ButtonDebouncer button(30);
  TEST_ASSERT_FALSE(button.poll(true, 0));
  TEST_ASSERT_FALSE(button.poll(false, 10));
  TEST_ASSERT_FALSE(button.poll(false, 39));
  TEST_ASSERT_TRUE(button.poll(false, 40));
  TEST_ASSERT_FALSE(button.poll(false, 100));
  TEST_ASSERT_TRUE(button.isPressed());
}

void test_bouncing_does_not_generate_multiple_presses(void) {
  poolanode::ButtonDebouncer button(30);
  TEST_ASSERT_FALSE(button.poll(false, 10));
  TEST_ASSERT_FALSE(button.poll(true, 20));
  TEST_ASSERT_FALSE(button.poll(false, 25));
  TEST_ASSERT_FALSE(button.poll(true, 35));
  TEST_ASSERT_FALSE(button.poll(false, 40));
  TEST_ASSERT_TRUE(button.poll(false, 70));
  TEST_ASSERT_FALSE(button.poll(false, 150));
}

void test_long_hold_does_not_repeat(void) {
  poolanode::ButtonDebouncer button(20);
  button.poll(false, 0);
  TEST_ASSERT_TRUE(button.poll(false, 20));
  TEST_ASSERT_FALSE(button.poll(false, 100));
  TEST_ASSERT_FALSE(button.poll(false, 1000));
}

void test_release_then_second_press_generates_second_event(void) {
  poolanode::ButtonDebouncer button(20);
  button.poll(false, 0);
  TEST_ASSERT_TRUE(button.poll(false, 20));
  button.poll(true, 30);
  TEST_ASSERT_FALSE(button.poll(true, 49));
  TEST_ASSERT_FALSE(button.poll(true, 50));
  button.poll(false, 60);
  TEST_ASSERT_TRUE(button.poll(false, 80));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_active_low_button_press_is_detected_once);
  RUN_TEST(test_bouncing_does_not_generate_multiple_presses);
  RUN_TEST(test_long_hold_does_not_repeat);
  RUN_TEST(test_release_then_second_press_generates_second_event);
  return UNITY_END();
}
