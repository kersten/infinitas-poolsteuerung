#include <unity.h>

#include "LedMapper.h"

void setUp(void) {}
void tearDown(void) {}

void test_swimming_pool_and_whirlpool_map_to_eight_led_halves(void) {
  poolanode::LedMapper mapper;
  for (uint8_t index = 0; index < poolanode::kLedsPerAnodeChannel; ++index) {
    TEST_ASSERT_EQUAL_UINT8(
        index, mapper.physicalIndex(poolanode::AnodeChannelId::SwimmingPool,
                                    index));
    TEST_ASSERT_EQUAL_UINT8(
        index + 8,
        mapper.physicalIndex(poolanode::AnodeChannelId::Whirlpool, index));
  }
}

void test_physical_led_order_is_configurable(void) {
  poolanode::LedMapperConfig config = {
      {7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8}, false, false};
  poolanode::LedMapper mapper(config);
  TEST_ASSERT_EQUAL_UINT8(
      7, mapper.physicalIndex(poolanode::AnodeChannelId::SwimmingPool, 0));
  TEST_ASSERT_EQUAL_UINT8(
      0, mapper.physicalIndex(poolanode::AnodeChannelId::SwimmingPool, 7));
  TEST_ASSERT_EQUAL_UINT8(
      15, mapper.physicalIndex(poolanode::AnodeChannelId::Whirlpool, 0));
}

void test_reversed_ring_orientation_is_configurable(void) {
  poolanode::LedMapperConfig config = {
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, true, false};
  poolanode::LedMapper mapper(config);
  TEST_ASSERT_EQUAL_UINT8(
      7, mapper.physicalIndex(poolanode::AnodeChannelId::SwimmingPool, 0));
  TEST_ASSERT_EQUAL_UINT8(
      0, mapper.physicalIndex(poolanode::AnodeChannelId::SwimmingPool, 7));
  TEST_ASSERT_EQUAL_UINT8(
      8, mapper.physicalIndex(poolanode::AnodeChannelId::Whirlpool, 0));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_swimming_pool_and_whirlpool_map_to_eight_led_halves);
  RUN_TEST(test_physical_led_order_is_configurable);
  RUN_TEST(test_reversed_ring_orientation_is_configurable);
  return UNITY_END();
}
