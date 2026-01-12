#pragma once

#include <atomic>

#include "i_player.hpp"

namespace ep::game
{
  class Player : public IPlayer {
  public:
    Player(std::size_t id, std::uint32_t hp,
           float x, float y, 
           float vel_x, float vel_y,
           std::uint8_t width, std::uint8_t height);
    ~Player() = default;

    std::size_t GetID() const noexcept override { return id_; }
    std::uint64_t GetAndIncrementSequenceID() noexcept override { return sequence_id_.fetch_add(1, std::memory_order_acq_rel); }
    float GetX() const noexcept override { return x_; }
    float GetY() const noexcept override { return y_; }
    std::uint32_t GetHP() const noexcept override { return hp_; };
    std::uint8_t GetWidth() const noexcept override { return width_; };
    std::uint8_t GetHeight() const noexcept override { return height_; };

    void Move(float dx, float dy) override;

    // velocity
    float GetVelX() const noexcept override { return vel_x_; }
    float GetVelY() const noexcept override { return vel_y_; }
    void SetVel(float vel_x, float vel_y) noexcept override { vel_x_ = vel_x; vel_y_ = vel_y; }

    bool OnGround() const noexcept override { return on_ground_; }
    void SetOnGround(bool state) noexcept override { on_ground_ = state; }

  private:
    std::size_t id_;
    std::atomic<std::uint64_t> sequence_id_;
    std::uint32_t hp_;
    float x_;
    float y_;
    float vel_x_;
    float vel_y_;
    std::uint8_t width_;
    std::uint8_t height_;
    bool on_ground_;
  };
}
