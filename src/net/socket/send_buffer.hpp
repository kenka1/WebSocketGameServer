#pragma once

#include <cstddef>
#include <memory>

namespace ep::net
{
  struct SendBuffer {
    std::size_t size_;
    std::shared_ptr<std::uint8_t[]> buf_;
  };
}
