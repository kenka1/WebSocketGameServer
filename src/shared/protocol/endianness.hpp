#pragma once

#if defined(_WIN32)
  #include <winsock2.h>
#elif defined(__linux__)
  #include <endian.h>
#else
  #error "Unsupported platform"
#endif

#include <bit>
#include <cstdint>
#include <cstring>

#if defined (_WIN32)
  inline std::uint16_t htobe16(std::uint16_t host_16bits)
  {
    return htons(host_16bits);
  }

  inline std::uint16_t be16toh(std::uint16_t net_16bits)
  {
    return htons(host_16bits);
  }

  inline std::uint32_t htobe32(std::uint32_t host_32bits)
  {
    return htonl(host_32bits);
  }

  inline std::uint32_t be32toh(std::uint32_t net_32bits)
  {
    return ntohl(host_32bits);
  }

  inline std::uint64_t htobe64(std::uint64_t host_64bits)
  {
    return htonll(host_64bits);
  }

  inline std::uint64_t be64toh(std::uint64_t net_64bits)
  {
    return ntohll(host_64bits);
  }

  inline std::uint32_t htonf32(float host_32bits)
  {
    return htonf(host_32bits);
  }

  inline float ntohf32(float net_32bits)
  {
    return ntohf(net_32bits);
  }
#elif defined (__linux__)
  inline std::uint32_t htonf32(float host_32bits)
  {
    std::uint32_t res;
    std::memcpy(&res, &host_32bits, sizeof(res));
    return htobe32(res);
  }
  
  inline float ntohf32(std::uint32_t net_32bits)
  {
    float res;
    net_32bits = be32toh(net_32bits);
    std::memcpy(&res, &net_32bits, sizeof(res));
    return res;
  }
  
  static_assert(sizeof(float) == sizeof(std::uint32_t));

#endif
