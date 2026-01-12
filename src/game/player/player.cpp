#include "player.hpp"

namespace ep::game
{
  Player::Player(std::size_t id, std::uint32_t hp,
                 float x, float y, 
                 float vel_x, float vel_y, 
                 std::uint8_t width, std::uint8_t height) :
    id_(id),
    sequence_id_(0),
    hp_(hp),
    x_(x),
    y_(y),
    vel_x_(vel_x),
    vel_y_(vel_y),
    width_(width),
    height_(height),
    on_ground_(false)
  {}

  void Player::Move(float dx, float dy)
  {
    x_ += dx;
    y_ += dy;
  }
}
