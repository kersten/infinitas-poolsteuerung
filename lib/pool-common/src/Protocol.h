#pragma once

#include <stddef.h>
#include <stdint.h>

#include "ChannelState.h"

namespace pool {
namespace protocol {

enum class Command : uint8_t {
  StartLeft,
  StopLeft,
  StartRight,
  StopRight,
  ToggleLeft,
  ToggleRight,
  Status,
  Unknown,
};

struct ParsedCommand {
  Command command;
  bool valid;
};

struct StateSnapshot {
  ChannelState left;
  ChannelState right;
  uint32_t leftRemainingMs;
  uint32_t rightRemainingMs;
};

// Commands and events are line based. Callers own the line buffer and should
// strip CR/LF before parsing. No dynamic allocation is used in the protocol.
ParsedCommand parseCommand(const char *line);
bool parseState(const char *line, StateSnapshot &state);

const char *commandName(Command command);
bool serializeState(char *buffer, size_t bufferSize, const StateSnapshot &state);
bool serializeError(char *buffer, size_t bufferSize, const char *code,
                    const char *message);

}  // namespace protocol
}  // namespace pool
