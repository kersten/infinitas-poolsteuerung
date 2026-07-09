#include <unity.h>

#include "LedMapper.h"

void setUp(void) {}
void tearDown(void) {}

void test_left_and_right_map_to_distinct_halves(void) {
  pool::LedMapper mapper;
  for (uint8_t i = 0; i < pool::kLedsPerChannel; ++i) {
    TEST_ASSERT_EQUAL_UINT8(i, mapper.physicalIndex(pool::ChannelId::Left, i));
    TEST_ASSERT_EQUAL_UINT8(i + 8,
                            mapper.physicalIndex(pool::ChannelId::Right, i));
  }
}

void test_physical_order_is_configurable(void) {
  pool::LedMapperConfig config = {
      {7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8}, false, false};
  pool::LedMapper mapper(config);
  TEST_ASSERT_EQUAL_UINT8(7, mapper.physicalIndex(pool::ChannelId::Left, 0));
  TEST_ASSERT_EQUAL_UINT8(0, mapper.physicalIndex(pool::ChannelId::Left, 7));
  TEST_ASSERT_EQUAL_UINT8(15, mapper.physicalIndex(pool::ChannelId::Right, 0));
}

void test_ring_orientation_can_be_reversed_per_half(void) {
  pool::LedMapperConfig config = {
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, true, false};
  pool::LedMapper mapper(config);
  TEST_ASSERT_EQUAL_UINT8(7, mapper.physicalIndex(pool::ChannelId::Left, 0));
  TEST_ASSERT_EQUAL_UINT8(0, mapper.physicalIndex(pool::ChannelId::Left, 7));
  TEST_ASSERT_EQUAL_UINT8(8, mapper.physicalIndex(pool::ChannelId::Right, 0));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_left_and_right_map_to_distinct_halves);
  RUN_TEST(test_physical_order_is_configurable);
  RUN_TEST(test_ring_orientation_can_be_reversed_per_half);
  return UNITY_END();
}
