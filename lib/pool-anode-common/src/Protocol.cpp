#include "Protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace poolanode {
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
  if (isExactCommand(line, "START_SWIMMING_POOL")) {
    return {Command::StartSwimmingPool, true};
  }
  if (isExactCommand(line, "STOP_SWIMMING_POOL")) {
    return {Command::StopSwimmingPool, true};
  }
  if (isExactCommand(line, "START_WHIRLPOOL")) {
    return {Command::StartWhirlpool, true};
  }
  if (isExactCommand(line, "STOP_WHIRLPOOL")) {
    return {Command::StopWhirlpool, true};
  }
  if (isExactCommand(line, "TOGGLE_SWIMMING_POOL")) {
    return {Command::ToggleSwimmingPool, true};
  }
  if (isExactCommand(line, "TOGGLE_WHIRLPOOL")) {
    return {Command::ToggleWhirlpool, true};
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
  char scratch[192];
  const size_t length = strlen(line);
  if (length >= sizeof(scratch)) {
    return false;
  }
  memcpy(scratch, line + 6, length - 5);

  bool gotSwimmingPool = false;
  bool gotWhirlpool = false;
  bool gotSwimmingPoolRemaining = false;
  bool gotWhirlpoolRemaining = false;
  char *save = 0;
  for (char *token = strtok_r(scratch, " ", &save); token != 0;
       token = strtok_r(0, " ", &save)) {
    const char *value = 0;
    if (parseKeyValue(token, "SWIMMING_POOL", value)) {
      if (!parseAnodeState(value, state.swimmingPool)) {
        return false;
      }
      gotSwimmingPool = true;
    } else if (parseKeyValue(token, "WHIRLPOOL", value)) {
      if (!parseAnodeState(value, state.whirlpool)) {
        return false;
      }
      gotWhirlpool = true;
    } else if (parseKeyValue(token, "SWIMMING_POOL_REMAINING", value)) {
      if (!parseUint32(value, state.swimmingPoolRemainingMs)) {
        return false;
      }
      gotSwimmingPoolRemaining = true;
    } else if (parseKeyValue(token, "WHIRLPOOL_REMAINING", value)) {
      if (!parseUint32(value, state.whirlpoolRemainingMs)) {
        return false;
      }
      gotWhirlpoolRemaining = true;
    } else {
      return false;
    }
  }
  return gotSwimmingPool && gotWhirlpool && gotSwimmingPoolRemaining &&
         gotWhirlpoolRemaining;
}

const char *commandName(Command command) {
  switch (command) {
    case Command::StartSwimmingPool:
      return "START_SWIMMING_POOL";
    case Command::StopSwimmingPool:
      return "STOP_SWIMMING_POOL";
    case Command::StartWhirlpool:
      return "START_WHIRLPOOL";
    case Command::StopWhirlpool:
      return "STOP_WHIRLPOOL";
    case Command::ToggleSwimmingPool:
      return "TOGGLE_SWIMMING_POOL";
    case Command::ToggleWhirlpool:
      return "TOGGLE_WHIRLPOOL";
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
      "STATE SWIMMING_POOL=%s WHIRLPOOL=%s SWIMMING_POOL_REMAINING=%lu "
      "WHIRLPOOL_REMAINING=%lu",
      anodeStateName(state.swimmingPool), anodeStateName(state.whirlpool),
      static_cast<unsigned long>(state.swimmingPoolRemainingMs),
      static_cast<unsigned long>(state.whirlpoolRemainingMs));
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
}  // namespace poolanode
