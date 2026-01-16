#pragma once

#include <optional>

#include "endianness.hpp"

namespace ep
{
  constexpr std::size_t GAME_PACKET_SIZE = 
    sizeof(std::uint32_t) * 3;

#pragma pack(push, 1)
  struct GamePacketNet {
    std::uint32_t hp_; // Player heath points
    std::uint32_t x_;  // Player world x position
    std::uint32_t y_;  // Player world y position
  };
#pragma pack(pop)

  struct GamePacket {
    std::uint32_t hp_;
    float x_;
    float y_;
  };

  static_assert(sizeof(GamePacketNet) == GAME_PACKET_SIZE);

  inline std::uint8_t* SerializeGamePacket(const GamePacket& packet, std::uint8_t* buf)
  {
    GamePacketNet g{};
  
    g.hp_ = htobe32(packet.hp_);
    g.x_  = htonf32(packet.x_);
    g.y_  = htonf32(packet.y_);

    std::memcpy(buf, &g, GAME_PACKET_SIZE);

    return buf;
  }

  inline std::optional<GamePacket> ParseGamePacket(std::uint8_t *data, std::size_t size)
  {
    if (size < GAME_PACKET_SIZE)
      return std::nullopt;

    GamePacketNet g{};
    std::memcpy(&g, data, GAME_PACKET_SIZE);  

    GamePacket packet{};
    packet.hp_ = be32toh(g.hp_);
    packet.x_  = ntohf32(g.x_);
    packet.y_  = ntohf32(g.y_);

    return std::optional<GamePacket>{packet};
  }
}
