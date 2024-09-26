cmake_minimum_required(VERSION 3.20)

project(hfl)

set(CMAKE_CXX_STANDARD 20)

list(
    APPEND HFL_INDLCUE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/memo.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/optional_function.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/result_function.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/result.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/timer.hpp"
)

add_library(hfl INTERFACE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vendor/googletest")

target_include_directories(
    hfl INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources(hfl INTERFACE ${HFL_INDLCUE_FILES})

source_group("include" FILES ${HFL_INDLCUE_FILES})

foreach(inc_file ${HFL_INDLCUE_FILES})
    message(STATUS  ${inc_file})
endforeach()

enable_testing()
include(GoogleTest)

add_executable(
  hfl_test
  "${CMAKE_CURRENT_SOURCE_DIR}/test/optional_fmap_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/optional_applicative_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/optional_mbind_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/result_fmap_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/result_applicative_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/result_mbind_test.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test/memo_test.cpp"

)
target_link_libraries(
  hfl_test PRIVATE
  GTest::gtest_main
  hfl
)


message(STATUS  "DONE")


