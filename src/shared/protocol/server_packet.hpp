#pragma once

#include <concepts>
#include <cstdint>

#include "protocol/game_packet.hpp"
#include "spdlog/spdlog.h"

#include "protocol/opcodes.hpp"
#include "protocol/packet_head.hpp"

namespace ep
{
  enum class ResponseType : uint8_t {
    Incoming,
    Broadcast,
    Rpc,
    RpcOthers,
  };

  template<typename T>
  concept PODType = std::is_standard_layout_v<T>;

  template<PODType T>
  class ServerPacket {
  public:
    ServerPacket(ResponseType type, std::size_t id, PacketHead head, std::optional<T> body = std::nullopt) :
      type_(type),
      id_(id),
      head_(head),
      body_(body)
    {}

    std::uint64_t GetID() const noexcept { return id_; }
    ResponseType GetResponseType() const noexcept { return type_; }
    PacketHead& GetPacketHead() noexcept { return head_; }
    std::optional<T>& GetBody() noexcept { return body_; }

    std::uint8_t* SerializePacket();

  private:
    ResponseType type_;
    std::uint64_t id_;
    PacketHead head_;
    std::optional<T> body_;
  };

  template<PODType T>
  std::uint8_t* ServerPacket<T>::SerializePacket()
  {
    switch (to_packet_type(head_.type_)) {
      case PacketType::PACKET_TYPE_GAME:
      {
        std::uint8_t* buf = new std::uint8_t[PACKET_HEAD_SIZE + GAME_PACKET_SIZE];
        SerializePacketHead(head_, buf);
        GamePacket& packet = static_cast<T>(body_.value());
        SerializeGamePacket(packet, buf + PACKET_HEAD_SIZE);
        return buf;
      }
      default:
        spdlog::warn("PacketType is not implemented");
    }
  }
}
