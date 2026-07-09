#include <unity.h>

#include "Protocol.h"

void setUp(void) {}
void tearDown(void) {}

void test_start_left_parses(void) {
  const pool::protocol::ParsedCommand parsed =
      pool::protocol::parseCommand("START_LEFT");
  TEST_ASSERT_TRUE(parsed.valid);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::protocol::Command::StartLeft),
                        static_cast<int>(parsed.command));
}

void test_stop_left_parses(void) {
  const pool::protocol::ParsedCommand parsed =
      pool::protocol::parseCommand("STOP_LEFT");
  TEST_ASSERT_TRUE(parsed.valid);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::protocol::Command::StopLeft),
                        static_cast<int>(parsed.command));
}

void test_status_parses(void) {
  const pool::protocol::ParsedCommand parsed = pool::protocol::parseCommand("STATUS");
  TEST_ASSERT_TRUE(parsed.valid);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::protocol::Command::Status),
                        static_cast<int>(parsed.command));
}

void test_unknown_command_returns_error(void) {
  const pool::protocol::ParsedCommand parsed =
      pool::protocol::parseCommand("TURN_EVERYTHING_ON");
  TEST_ASSERT_FALSE(parsed.valid);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pool::protocol::Command::Unknown),
                        static_cast<int>(parsed.command));
}

void test_state_serializes_and_parses(void) {
  const pool::protocol::StateSnapshot state = {
      pool::ChannelState::Starting, pool::ChannelState::Running, 1800000, 7654321};
  char line[144];
  TEST_ASSERT_TRUE(pool::protocol::serializeState(line, sizeof(line), state));
  TEST_ASSERT_EQUAL_STRING(
      "STATE LEFT=STARTING RIGHT=RUNNING LEFT_REMAINING=1800000 "
      "RIGHT_REMAINING=7654321",
      line);

  pool::protocol::StateSnapshot parsed;
  TEST_ASSERT_TRUE(pool::protocol::parseState(line, parsed));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(state.left), static_cast<int>(parsed.left));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(state.right), static_cast<int>(parsed.right));
  TEST_ASSERT_EQUAL_UINT32(state.leftRemainingMs, parsed.leftRemainingMs);
  TEST_ASSERT_EQUAL_UINT32(state.rightRemainingMs, parsed.rightRemainingMs);
}

void test_malformed_messages_do_not_parse(void) {
  pool::protocol::StateSnapshot state;
  TEST_ASSERT_FALSE(pool::protocol::parseState("STATE LEFT=IDLE", state));
  TEST_ASSERT_FALSE(pool::protocol::parseState(
      "STATE LEFT=BOGUS RIGHT=IDLE LEFT_REMAINING=0 RIGHT_REMAINING=0", state));
  TEST_ASSERT_FALSE(pool::protocol::parseState(
      "STATE LEFT=IDLE RIGHT=IDLE LEFT_REMAINING=nope RIGHT_REMAINING=0", state));
  TEST_ASSERT_FALSE(pool::protocol::parseState(0, state));
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(test_start_left_parses);
  RUN_TEST(test_stop_left_parses);
  RUN_TEST(test_status_parses);
  RUN_TEST(test_unknown_command_returns_error);
  RUN_TEST(test_state_serializes_and_parses);
  RUN_TEST(test_malformed_messages_do_not_parse);
  return UNITY_END();
}
