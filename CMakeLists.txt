cmake_minimum_required(VERSION 3.15)
project(tote
  VERSION 1.0
  DESCRIPTION "A brief, lightweight C++ library for containers such as resizable arrays and hash maps."
  LANGUAGES CXX
)

function(download_cpm)
  if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake")
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD "https://github.com/TheLartians/CPM.cmake/releases/latest/download/cpm.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake")
  endif()
  include("${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake")
endfunction()

download_cpm()

CPMAddPackage("gh:onqtam/doctest#2.4.6")

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  add_executable(${CMAKE_PROJECT_NAME})
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
  target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PUBLIC "${doctest_SOURCE_DIR}")
  add_subdirectory(tests)
  if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT {$CMAKE_PROJECT_NAME})
  endif()
else()
  add_library(${PROJECT_NAME})
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Weverything -Wno-c++98-c++11-c++14-compat -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++20-compat>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)
target_include_directories(${PROJECT_NAME}
  PUBLIC
  "include"
)
add_subdirectory(src)
