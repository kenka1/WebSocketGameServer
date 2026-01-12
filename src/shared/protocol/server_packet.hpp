#pragma once

#include <concepts>
#include <cstdint>

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
    ResponseType GetType() const noexcept { return type_; }
    PacketHead& GetPacketHead() noexcept { return head_; }
    std::optional<T>& GetBody() noexcept { return body_; }

  private:
    ResponseType type_;
    std::uint64_t id_;
    PacketHead head_;
    std::optional<T> body_;
  };
}
