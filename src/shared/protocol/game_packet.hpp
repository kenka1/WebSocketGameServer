#pragma once

#include <optional>

#include "endianness.hpp"

namespace ep
{
  constexpr std::size_t GAME_PACKET_SIZE = 
    sizeof(std::uint32_t) +
    sizeof(float) * 2;

#pragma pack(push, 1)
  struct GamePacket {
    std::uint32_t hp_; // Player heath points
    float         x_;  // Player world x position
    float         y_;  // Player world y position
  };
#pragma pack(pop)

  static_assert(sizeof(GamePacket) == GAME_PACKET_SIZE);

  inline std::uint8_t* SerializeGamePacket(GamePacket& packet)
  {
    std::uint8_t* data = new std::uint8_t[GAME_PACKET_SIZE];
    if (!data)
      return nullptr;

    if constexpr (std::endian::native != std::endian::big) {
      GamePacket g{};
    
      g.hp_ = htobe32(packet.hp_);
      g.x_  = htonf32(packet.x_);
      g.y_  = htonf32(packet.y_);

      std::memcpy(data, &g, GAME_PACKET_SIZE);
    } else {
      std::memcpy(data, &packet, GAME_PACKET_SIZE);
    }

    return data;
  }

  inline std::optional<GamePacket> ParseGamePacket(std::uint8_t *data, std::size_t size)
  {
    if (size < GAME_PACKET_SIZE)
      return std::nullopt;

    GamePacket g{};
    std::memcpy(&g, data, GAME_PACKET_SIZE);  

    if constexpr (std::endian::native != std::endian::big) {
      g.hp_ = htole32(g.hp_);
      g.x_  = ntohf32(g.x_);
      g.y_  = ntohf32(g.y_);
    }

    return std::optional<GamePacket>{g};
  }
}
