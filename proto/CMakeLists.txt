find_package(Protobuf REQUIRED)

set(PROTO_FILES
  protocol.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(protocol STATIC
  ${PROTO_SRCS}
  ${PROTO_HDRS}
)

add_library(protocol::protocol ALIAS 
  protocol
)

target_link_libraries(protocol PUBLIC
  protobuf::libprotobuf
)

target_include_directories(protocol PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)
