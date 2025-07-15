cmake_minimum_required(VERSION 3.10)
project(MindBlowingGame C)

# Need this to make SDLTTF work
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

# Source and include directories
file(GLOB SOURCES "src/*.c")
include_directories("./src/include")

# Add SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# And SDL2 fonts
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})

# Executable
add_executable(GOATGame ${SOURCES})
target_link_libraries(GOATGame PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    m # math lib
)

# Copy assets into build folder after build
add_custom_command(TARGET GOATGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:GOATGame>/assets)
