#pragma once

#include <stddef.h>
#include <stdint.h>

#include "AnodeState.h"

namespace poolanode {
namespace protocol {

enum class Command : uint8_t {
  StartSwimmingPool,
  StopSwimmingPool,
  StartWhirlpool,
  StopWhirlpool,
  ToggleSwimmingPool,
  ToggleWhirlpool,
  Status,
  Unknown,
};

struct ParsedCommand {
  Command command;
  bool valid;
};

struct StateSnapshot {
  AnodeState swimmingPool;
  AnodeState whirlpool;
  uint32_t swimmingPoolRemainingMs;
  uint32_t whirlpoolRemainingMs;
};

ParsedCommand parseCommand(const char *line);
bool parseState(const char *line, StateSnapshot &state);
const char *commandName(Command command);
bool serializeState(char *buffer, size_t bufferSize, const StateSnapshot &state);
bool serializeError(char *buffer, size_t bufferSize, const char *code,
                    const char *message);

}  // namespace protocol
}  // namespace poolanode
