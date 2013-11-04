cmake_minimum_required(VERSION 2.8)
project(eatft)

set(eatft_SRCS
  src/eatft.c
  src/protocol.c
  src/widgets.c
  src/button.c
  src/switch.c
)

if (UNIX)
  set(eatft_SRCS
    ${eatft_SRCS}
    src/unix.c
    )
endif()

include_directories("./include")

add_library(eatft
  ${eatft_SRCS}
)

add_executable(test
  ./test/test.c
  ./test/test_unix.c
)

target_link_libraries(test eatft)
