cmake_minimum_required(VERSION 3.10)
project(CrimsonShells C)

# Need this to make SDLTTF work
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

# Sources are all located in src/
file(GLOB_RECURSE SOURCES "src/*.c")

# SDL2 with extensions
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Executable
add_executable(crimshells ${SOURCES})

# Set paths
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

target_include_directories(crimshells PRIVATE
	${SOURCE_DIR}
	${SDL2_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
)

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
    ${CMAKE_SOURCE_DIR}/settings.ini ${CMAKE_BINARY_DIR}/settings.ini

    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data

    COMMENT "Creating symbolic links..."
)

# Make the main target depend on the assets symlink
add_dependencies(crimshells symlinks)
