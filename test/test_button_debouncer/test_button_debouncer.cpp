#include <unity.h>

#include "ButtonDebouncer.h"

void setUp(void) {}
void tearDown(void) {}

void test_active_low_press_is_detected_once(void) {
  pool::ButtonDebouncer button(30);
  TEST_ASSERT_FALSE(button.poll(true, 0));
  TEST_ASSERT_FALSE(button.poll(false, 10));
  TEST_ASSERT_FALSE(button.poll(false, 39));
  TEST_ASSERT_TRUE(button.poll(false, 40));
  TEST_ASSERT_FALSE(button.poll(false, 100));
  TEST_ASSERT_TRUE(button.isPressed());
}

void test_bounce_does_not_create_multiple_presses(void) {
  pool::ButtonDebouncer button(30);
  TEST_ASSERT_FALSE(button.poll(false, 10));
  TEST_ASSERT_FALSE(button.poll(true, 20));
  TEST_ASSERT_FALSE(button.poll(false, 25));
  TEST_ASSERT_FALSE(button.poll(true, 35));
  TEST_ASSERT_FALSE(button.poll(false, 40));
  TEST_ASSERT_TRUE(button.poll(false, 70));
  TEST_ASSERT_FALSE(button.poll(false, 150));
}

void test_long_hold_does_not_repeat(void) {
  pool::ButtonDebouncer button(20);
  button.poll(false, 0);
  TEST_ASSERT_TRUE(button.poll(false, 20));
  TEST_ASSERT_FALSE(button.poll(false, 100));
  TEST_ASSERT_FALSE(button.poll(false, 1000));
}

void test_release_then_second_press_generates_second_event(void) {
  pool::ButtonDebouncer button(20);
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
  RUN_TEST(test_active_low_press_is_detected_once);
  RUN_TEST(test_bounce_does_not_create_multiple_presses);
  RUN_TEST(test_long_hold_does_not_repeat);
  RUN_TEST(test_release_then_second_press_generates_second_event);
  return UNITY_END();
}
