#include <gtest/gtest.h>

#include "protocol/game_packet.hpp"

TEST(GamePacketTests, SerializeParse)
{
  ep::GamePacket packet{100, 3.141592, 2.718281};

  auto buf = std::make_unique<std::uint8_t[]>(ep::GAME_PACKET_SIZE);
  ep::SerializeGamePacket(packet, buf.get());

  auto parsed_packet = ep::ParseGamePacket(buf.get(), ep::GAME_PACKET_SIZE);
  EXPECT_NE(parsed_packet, std::nullopt);

  EXPECT_EQ(packet.hp_, parsed_packet->hp_);
  EXPECT_EQ(packet.x_, parsed_packet->x_);
  EXPECT_EQ(packet.y_, parsed_packet->y_);
}
