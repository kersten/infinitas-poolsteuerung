#include "Protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace pool {
namespace protocol {
namespace {

bool isExactCommand(const char *line, const char *command) {
  return line != 0 && strcmp(line, command) == 0;
}

bool parseUint32(const char *text, uint32_t &value) {
  if (text == 0 || *text == '\0') {
    return false;
  }
  char *end = 0;
  const unsigned long parsed = strtoul(text, &end, 10);
  if (*end != '\0' || parsed > 0xFFFFFFFFUL) {
    return false;
  }
  value = static_cast<uint32_t>(parsed);
  return true;
}

bool parseKeyValue(const char *token, const char *key, const char *&value) {
  const size_t keyLength = strlen(key);
  if (strncmp(token, key, keyLength) != 0 || token[keyLength] != '=') {
    return false;
  }
  value = token + keyLength + 1;
  return *value != '\0';
}

}  // namespace

ParsedCommand parseCommand(const char *line) {
  const ParsedCommand unknown = {Command::Unknown, false};
  if (line == 0) {
    return unknown;
  }
  if (isExactCommand(line, "START_LEFT")) {
    return {Command::StartLeft, true};
  }
  if (isExactCommand(line, "STOP_LEFT")) {
    return {Command::StopLeft, true};
  }
  if (isExactCommand(line, "START_RIGHT")) {
    return {Command::StartRight, true};
  }
  if (isExactCommand(line, "STOP_RIGHT")) {
    return {Command::StopRight, true};
  }
  if (isExactCommand(line, "TOGGLE_LEFT")) {
    return {Command::ToggleLeft, true};
  }
  if (isExactCommand(line, "TOGGLE_RIGHT")) {
    return {Command::ToggleRight, true};
  }
  if (isExactCommand(line, "STATUS")) {
    return {Command::Status, true};
  }
  return unknown;
}

bool parseState(const char *line, StateSnapshot &state) {
  if (line == 0 || strncmp(line, "STATE ", 6) != 0) {
    return false;
  }

  // The gateway has a bounded UART line buffer, so a fixed scratch copy keeps
  // parsing deterministic and avoids allocations on embedded targets.
  char scratch[160];
  const size_t length = strlen(line);
  if (length >= sizeof(scratch)) {
    return false;
  }
  memcpy(scratch, line + 6, length - 5);

  bool gotLeft = false;
  bool gotRight = false;
  bool gotLeftRemaining = false;
  bool gotRightRemaining = false;
  char *save = 0;
  for (char *token = strtok_r(scratch, " ", &save); token != 0;
       token = strtok_r(0, " ", &save)) {
    const char *value = 0;
    if (parseKeyValue(token, "LEFT", value)) {
      if (!parseChannelState(value, state.left)) {
        return false;
      }
      gotLeft = true;
    } else if (parseKeyValue(token, "RIGHT", value)) {
      if (!parseChannelState(value, state.right)) {
        return false;
      }
      gotRight = true;
    } else if (parseKeyValue(token, "LEFT_REMAINING", value)) {
      if (!parseUint32(value, state.leftRemainingMs)) {
        return false;
      }
      gotLeftRemaining = true;
    } else if (parseKeyValue(token, "RIGHT_REMAINING", value)) {
      if (!parseUint32(value, state.rightRemainingMs)) {
        return false;
      }
      gotRightRemaining = true;
    } else {
      return false;
    }
  }
  return gotLeft && gotRight && gotLeftRemaining && gotRightRemaining;
}

const char *commandName(Command command) {
  switch (command) {
    case Command::StartLeft:
      return "START_LEFT";
    case Command::StopLeft:
      return "STOP_LEFT";
    case Command::StartRight:
      return "START_RIGHT";
    case Command::StopRight:
      return "STOP_RIGHT";
    case Command::ToggleLeft:
      return "TOGGLE_LEFT";
    case Command::ToggleRight:
      return "TOGGLE_RIGHT";
    case Command::Status:
      return "STATUS";
    case Command::Unknown:
      return "UNKNOWN";
  }
  return "UNKNOWN";
}

bool serializeState(char *buffer, size_t bufferSize, const StateSnapshot &state) {
  if (buffer == 0 || bufferSize == 0) {
    return false;
  }
  const int result = snprintf(
      buffer, bufferSize,
      "STATE LEFT=%s RIGHT=%s LEFT_REMAINING=%lu RIGHT_REMAINING=%lu",
      channelStateName(state.left), channelStateName(state.right),
      static_cast<unsigned long>(state.leftRemainingMs),
      static_cast<unsigned long>(state.rightRemainingMs));
  return result >= 0 && static_cast<size_t>(result) < bufferSize;
}

bool serializeError(char *buffer, size_t bufferSize, const char *code,
                    const char *message) {
  if (buffer == 0 || bufferSize == 0 || code == 0 || message == 0) {
    return false;
  }
  const int result = snprintf(buffer, bufferSize, "ERROR CODE=%s MESSAGE=%s",
                              code, message);
  return result >= 0 && static_cast<size_t>(result) < bufferSize;
}

}  // namespace protocol
}  // namespace pool
