add_library(shared
  config/config.cpp
  protocol/net_packet.cpp
  protocol/protocol.cpp
)

target_include_directories(shared PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(shared PRIVATE
  protocol
)

target_include_directories(shared PUBLIC
  ${CMAKE_SOURCE_DIR}/src/shared
)
