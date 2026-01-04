#pragma once

#include <cstdint>
#include <vector>

#define PROTOCOL_VERSION 1

namespace ep
{
  std::vector<std::uint8_t> PlayerStatePacket(std::uint64_t player_id, std::uint64_t request_id, float x, float y);
}
