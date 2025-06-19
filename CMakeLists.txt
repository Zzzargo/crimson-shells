cmake_minimum_required(VERSION 3.10)
project(MindBlowingGame C)

set(CMAKE_C_STANDARD 99)

# Source and include directories
file(GLOB SOURCES "src/*.c")
include_directories("./src/include")

# Add SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Executable
add_executable(GOATGame ${SOURCES})
target_link_libraries(GOATGame ${SDL2_LIBRARIES})

# Copy assets into build folder after build
add_custom_command(TARGET GOATGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:GOATGame>/assets)
