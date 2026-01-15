#pragma once

#include <optional>

#include "endianness.hpp"

namespace ep
{
  constexpr std::size_t PACKET_HEAD_SIZE = 
    sizeof(std::uint16_t) * 2 +
    sizeof(std::uint8_t)      +
    sizeof(std::uint32_t)     +
    sizeof(std::uint64_t) * 2;

#pragma pack(push, 1)
  struct PacketHead {
    std::uint16_t version_;     // Protocol version
    std::uint16_t opcode_;      // Packet opcode
    std::uint8_t  type_;        // Packet type
    std::uint32_t size_;        // Total packet size (head + body)
    std::uint64_t tick_;        // Time stamp
    std::uint64_t sequence_id_; // Packet sequence id
  };
#pragma pack(pop)

  static_assert(sizeof(PacketHead) == PACKET_HEAD_SIZE);

  inline std::uint8_t* SerializePacketHead(const PacketHead& packet, std::uint8_t* buf)
  {
    if constexpr (std::endian::native != std::endian::big) {
      PacketHead h{};

      h.version_     = htobe16(packet.version_);
      h.opcode_      = htobe16(packet.opcode_);
      h.type_        = packet.type_;
      h.size_        = htobe32(packet.size_);
      h.tick_        = htobe64(packet.tick_);
      h.sequence_id_ = htobe64(packet.sequence_id_);

      std::memcpy(buf, &h, PACKET_HEAD_SIZE);
    } else {
      std::memcpy(buf, &packet, PACKET_HEAD_SIZE);
    }

    return buf;
  }

  inline std::optional<PacketHead> ParsePacketHead(std::uint8_t *data, std::size_t size)
  {
    if (size < PACKET_HEAD_SIZE)
      return std::nullopt;

    PacketHead h{};
    std::memcpy(&h, data, PACKET_HEAD_SIZE);  

    if constexpr (std::endian::native != std::endian::big) {
      h.version_     = be16toh(h.version_);
      h.opcode_      = be16toh(h.opcode_);
      h.type_        = h.type_;
      h.size_        = be32toh(h.size_);
      h.tick_        = be64toh(h.tick_);
      h.sequence_id_ = be64toh(h.sequence_id_);
    }

    return std::optional<PacketHead>{h};
  }
}
