cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    gameboy-emulator
    VERSION 0.1.0
    DESCRIPTION "C++ emulator for the original game boy"
    HOMEPAGE_URL "https://th00p.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    gameboy-emulator_lib OBJECT
    source/emulator.cpp
    source/bus.cpp
    source/cart.cpp
    source/cpu.cpp
    source/instructions.cpp
    source/ram.cpp
    source/ui.cpp)

target_include_directories(
    gameboy-emulator_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(gameboy-emulator_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
target_link_libraries(gameboy-emulator_lib PUBLIC fmt::fmt)

find_package(SDL2 REQUIRED)
target_link_libraries(gameboy-emulator_lib PUBLIC SDL2::SDL2)

find_package(SDL2_ttf REQUIRED)
target_link_libraries(gameboy-emulator_lib PUBLIC $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)

# ---- Declare executable ----

add_executable(gameboy-emulator_exe source/main.cpp)
add_executable(gameboy-emulator::exe ALIAS gameboy-emulator_exe)

set_property(TARGET gameboy-emulator_exe PROPERTY OUTPUT_NAME gameboy-emulator)

target_compile_features(gameboy-emulator_exe PRIVATE cxx_std_17)

target_link_libraries(gameboy-emulator_exe PRIVATE gameboy-emulator_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT gameboy-emulator_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of gameboy-emulator"
  )
endif()

include(cmake/dev-mode.cmake)
