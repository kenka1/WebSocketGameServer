#pragma once

#include <cstdint>

namespace ep
{
#pragma pack(push, 1)
    struct GamePacket{
      std::uint64_t player_id_;
      float x_;
      float y_;
    };
#pragma pack(pop)
}
