#include <iostream>

#include <gtest/gtest.h>

#include "protocol/packet_head.hpp"

TEST(PacketHeadTest, SerializeParse)
{
  ep::PacketHead packet{1, 2, 3, 4, 5, 6};

  auto buf = std::make_unique<std::uint8_t[]>(ep::PACKET_HEAD_SIZE);
  ep::SerializePacketHead(packet, buf.get());

  auto parsed_packet = ep::ParsePacketHead(buf.get(), ep::PACKET_HEAD_SIZE);
  EXPECT_NE(parsed_packet, std::nullopt);

  EXPECT_EQ(packet.version_, parsed_packet->version_);
  EXPECT_EQ(packet.opcode_, parsed_packet->opcode_);
  EXPECT_EQ(packet.type_, parsed_packet->type_);
  EXPECT_EQ(packet.size_, parsed_packet->size_);
  EXPECT_EQ(packet.tick_, parsed_packet->tick_);
  EXPECT_EQ(packet.sequence_id_, parsed_packet->sequence_id_);
}
