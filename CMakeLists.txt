cmake_minimum_required(VERSION 3.10)
project(CrimsonShells C)

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

# Image
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

# And sounds
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_MIXER_INCLUDE_DIRS})

# Executable
add_executable(crimshells ${SOURCES})
target_link_libraries(crimshells PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    m # math lib
)

# Create symlinks to resources
add_custom_target(symlinks ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets

    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/arenatest ${CMAKE_BINARY_DIR}/arenatest

    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/settings.ini ${CMAKE_BINARY_DIR}/settings.ini
    COMMENT "Creating symbolic links..."
)

# Make the main target depend on the assets symlink
add_dependencies(crimshells symlinks)