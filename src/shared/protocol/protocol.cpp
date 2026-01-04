#include "protocol.h"

#include <netinet/in.h>

#include <cstdint>
#include <algorithm>

#include "protocol.pb.h"

namespace protocol = game::protocol::v1;

namespace ep
{
  template<typename T>
  static T swap_endian(T value)
  {
    std::uint8_t* ptr = reinterpret_cast<std::uint8_t*>(&value);
    std::reverse(ptr, ptr + sizeof(T));
    return *reinterpret_cast<T*>(ptr);
  }

  std::vector<std::uint8_t> SerializePlayerState(std::uint64_t player_id, std::uint64_t request_id, float x, float y)
  {
    protocol::PacketHeader head;
    head.set_opcode(protocol::OPCODE_PLAYER_STATE);
    head.set_version(PROTOCOL_VERSION);
    head.set_request_id(request_id);


    protocol::PlayerState body;
    body.set_player_id(player_id);
    auto* pos = body.mutable_position();
    pos->set_x(x);
    pos->set_y(y);

    const std::size_t header_size = head.ByteSizeLong();
    const std::size_t body_size = body.ByteSizeLong();
    const std::size_t packet_size = header_size + packet_size;

    std::vector<std::uint8_t> buf(sizeof(std::uint32_t) + packet_size);

    std::uint32_t len = htonl(packet_size);
    memcpy(buf.data(), &len, sizeof(len));

    std::uint8_t* ptr = buf.data() + sizeof(len);
    head.SerializeToArray(ptr, header_size);
    ptr += header_size;
    body.SerializeToArray(ptr, body_size);

    return buf;
  }
}
