find_package(GTest QUIET)

if (NOT GTest_FOUND)
  message(STATUS "GTest not found — fetching from GitHub...")

  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
  )

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)
else()
  message(STATUS "Using system-installed GTest")
endif()

set(SOURCES
  main.cpp

  # src/shared
  shared/utils/ts_queue_tests.cpp
  shared/protocol/packet_head_tests.cpp
  shared/protocol/game_packet_tests.cpp

  # src/game
  game/physics/collision_tests.cpp
  game/physics/swept_math_tests.cpp
  cryptography/sha256_tests.cpp
)

add_executable(unit_tests 
  ${SOURCES}
)

target_link_libraries(unit_tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  GTest::gmock
  net
  game
  crypto
)

target_include_directories(unit_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src/net
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/tests/unit
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
