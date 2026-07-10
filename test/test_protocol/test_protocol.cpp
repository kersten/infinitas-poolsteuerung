#include <unity.h>

#include "Protocol.h"

void setUp(void) {}
void tearDown(void) {}

void test_swimming_pool_commands_parse(void) {
  const poolanode::protocol::ParsedCommand start =
      poolanode::protocol::parseCommand("START_SWIMMING_POOL");
  const poolanode::protocol::ParsedCommand stop =
      poolanode::protocol::parseCommand("STOP_SWIMMING_POOL");
  TEST_ASSERT_TRUE(start.valid);
  TEST_ASSERT_TRUE(stop.valid);
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::protocol::Command::StartSwimmingPool),
      static_cast<int>(start.command));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::protocol::Command::StopSwimmingPool),
      static_cast<int>(stop.command));
}

void test_whirlpool_commands_parse(void) {
  const poolanode::protocol::ParsedCommand start =
      poolanode::protocol::parseCommand("START_WHIRLPOOL");
  const poolanode::protocol::ParsedCommand stop =
      poolanode::protocol::parseCommand("STOP_WHIRLPOOL");
  TEST_ASSERT_TRUE(start.valid);
  TEST_ASSERT_TRUE(stop.valid);
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::protocol::Command::StartWhirlpool),
      static_cast<int>(start.command));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::protocol::Command::StopWhirlpool),
      static_cast<int>(stop.command));
}

void test_status_parses(void) {
  const poolanode::protocol::ParsedCommand parsed =
      poolanode::protocol::parseCommand("STATUS");
  TEST_ASSERT_TRUE(parsed.valid);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(poolanode::protocol::Command::Status),
                        static_cast<int>(parsed.command));
}

void test_unknown_command_returns_error(void) {
  const poolanode::protocol::ParsedCommand parsed =
      poolanode::protocol::parseCommand("TURN_EVERYTHING_ON");
  TEST_ASSERT_FALSE(parsed.valid);
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(poolanode::protocol::Command::Unknown),
      static_cast<int>(parsed.command));
}

void test_state_serializes_and_parses(void) {
  const poolanode::protocol::StateSnapshot state = {
      poolanode::AnodeState::Starting, poolanode::AnodeState::Running,
      7200000, 1234567};
  char line[180];
  TEST_ASSERT_TRUE(poolanode::protocol::serializeState(line, sizeof(line),
                                                        state));
  TEST_ASSERT_EQUAL_STRING(
      "STATE SWIMMING_POOL=STARTING WHIRLPOOL=RUNNING "
      "SWIMMING_POOL_REMAINING=7200000 WHIRLPOOL_REMAINING=1234567",
      line);

  poolanode::protocol::StateSnapshot parsed;
  TEST_ASSERT_TRUE(poolanode::protocol::parseState(line, parsed));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(state.swimmingPool),
                        static_cast<int>(parsed.swimmingPool));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(state.whirlpool),
                        static_cast<int>(parsed.whirlpool));
  TEST_ASSERT_EQUAL_UINT32(state.swimmingPoolRemainingMs,
                           parsed.swimmingPoolRemainingMs);
  TEST_ASSERT_EQUAL_UINT32(state.whirlpoolRemainingMs,
                           parsed.whirlpoolRemainingMs);
}

void test_malformed_messages_do_not_crash_or_parse(void) {
  poolanode::protocol::StateSnapshot state;
  TEST_ASSERT_FALSE(poolanode::protocol::parseState(
      "STATE SWIMMING_POOL=IDLE", state));
  TEST_ASSERT_FALSE(poolanode::protocol::parseState(
      "STATE SWIMMING_POOL=BOGUS WHIRLPOOL=IDLE "
      "SWIMMING_POOL_REMAINING=0 WHIRLPOOL_REMAINING=0",
      state));
  TEST_ASSERT_FALSE(poolanode::protocol::parseState(
      "STATE SWIMMING_POOL=IDLE WHIRLPOOL=IDLE "
      "SWIMMING_POOL_REMAINING=nope WHIRLPOOL_REMAINING=0",
      state));
  TEST_ASSERT_FALSE(poolanode::protocol::parseState(0, state));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_swimming_pool_commands_parse);
  RUN_TEST(test_whirlpool_commands_parse);
  RUN_TEST(test_status_parses);
  RUN_TEST(test_unknown_command_returns_error);
  RUN_TEST(test_state_serializes_and_parses);
  RUN_TEST(test_malformed_messages_do_not_crash_or_parse);
  return UNITY_END();
}
