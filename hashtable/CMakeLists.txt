cmake_minimum_required(VERSION 3.20)
project(hashtables C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd (neovim LSP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(${PROJECT_NAME} set.c)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Wpedantic
)

