# FindSDL2_image.cmake
find_path(SDL2_IMAGE_INCLUDE_DIRS
    NAMES SDL_image.h
    PATHS /usr/include/SDL2 /usr/local/include/SDL2
)

find_library(SDL2_IMAGE_LIBRARIES
    NAMES SDL2_image
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_image
    REQUIRED_VARS SDL2_IMAGE_LIBRARIES SDL2_IMAGE_INCLUDE_DIRS
)

mark_as_advanced(SDL2_IMAGE_INCLUDE_DIRS SDL2_IMAGE_LIBRARIES)
