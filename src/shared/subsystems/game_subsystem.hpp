#pragma once

#include "protocol/game_packet.hpp"
#include "protocol/packet_head.hpp"
#include "protocol/server_packet.hpp"
#include "utils/ts_queue.hpp"

namespace ep
{
  class GameSubsystem {
  public:
    GameSubsystem() = default;
    ~GameSubsystem() = default;
    GameSubsystem(const GameSubsystem&) = delete;
    GameSubsystem& operator=(const GameSubsystem&) = delete;

    TSQueue<ServerPacket<PacketHead>> in_queue_;
    TSQueue<ServerPacket<GamePacket>> out_queue_;
  };
}
