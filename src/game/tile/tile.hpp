#pragma once

#include "tile/i_box.hpp"

namespace ep::game
{
  enum class TileType : std::uint16_t {
    Empty,
    Solid,
  };

  class Tile : public IBox {
  public:
    Tile() : type_(TileType::Empty) {}
    explicit Tile(float x, float y, std::uint8_t width, std::uint8_t height, TileType type) noexcept :
      x_(x), y_(y), width_(width), height_(height),type_(type)
    {}

    float GetX() const noexcept override { return x_; }
    float GetY() const noexcept override { return y_; }
    std::uint8_t GetWidth() const noexcept override { return width_; }
    std::uint8_t GetHeight() const noexcept override { return height_; }
    TileType GetType() const noexcept { return type_; }
  private:
    float x_;
    float y_;
    std::uint8_t width_;
    std::uint8_t height_;
    TileType type_;
  };

}
