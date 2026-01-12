#pragma once

#include "tile/i_box.hpp"

namespace ep::game
{
  class IPlayer : public IBox{
  public:
    virtual ~IPlayer() = default;

    virtual std::size_t GetID() const noexcept = 0;
    virtual std::uint64_t GetAndIncrementSequenceID() noexcept = 0;
    virtual float GetVelX() const noexcept = 0;
    virtual float GetVelY() const noexcept = 0;
    virtual std::uint32_t GetHP() const noexcept = 0;
    virtual void Move(float x, float y) = 0;
    virtual void SetVel(float vel_x, float vel_y) noexcept = 0;
    virtual bool OnGround() const noexcept = 0;
    virtual void SetOnGround(bool state) noexcept = 0;
  };
}
