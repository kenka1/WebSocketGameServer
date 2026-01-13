#pragma once

#include <cstdint>
#include <cstring>

namespace ep
{
  enum class Opcodes : std::uint16_t {
    // =============================
    //  AUTH 
    // =============================
    OPCODE_AUTH_REQUEST        = 0x0101,
    OPCODE_AUTH_RESPONSE       = 0x0102,

    // =============================
    // GAME
    // =============================
    OPCODE_ADD_PLAYER          = 0x0201,
    OPCODE_CREATE_PLAYER       = 0x0202,
    OPCODE_SPAWN_PLAYERS       = 0x0203,
    OPCODE_REMOVE_PLAYER       = 0x0204,
    OPCODE_PLAYER_STATE        = 0x0205,

    // =============================
    //  CLIENT INPUT 
    // =============================
    OPCODE_LEFT_MOVE            = 0x0301,
    OPCODE_RIGHT_MOVE           = 0x0302,
    OPCODE_JUMP                 = 0x0303,
    OPCODE_LEFT_JUMP            = 0x0304,
    OPCODE_RIGHT_JUMP           = 0x0305,
  };

  enum class PacketType : std::uint8_t {
    PACKET_TYPE_AUTH       = 0x01,
    PACKET_TYPE_GAME       = 0x02,
    PACKET_TYPE_GAME_INPUT = 0x03,
  };

  inline std::uint16_t opcode_to_uint16(Opcodes opcode)
  {
    return static_cast<std::uint16_t>(opcode);
  }

  inline Opcodes to_opcode(std::uint16_t opcode)
  {
    return static_cast<Opcodes>(opcode);
  }

  inline std::uint8_t packet_type_to_uint8(PacketType opcode)
  {
    return static_cast<std::uint8_t>(opcode);
  }

  inline PacketType to_packet_type(std::uint8_t opcode)
  {
    return static_cast<PacketType>(opcode);
  }
}
