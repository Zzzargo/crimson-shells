# FindSDL2_mixer.cmake
find_path(SDL2_MIXER_INCLUDE_DIRS
    NAMES SDL_mixer.h
    PATHS /usr/include/SDL2 /usr/local/include/SDL2
)

find_library(SDL2_MIXER_LIBRARIES
    NAMES SDL2_mixer
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer
    REQUIRED_VARS SDL2_MIXER_LIBRARIES SDL2_MIXER_INCLUDE_DIRS
)

mark_as_advanced(SDL2_MIXER_INCLUDE_DIRS SDL2_MIXER_LIBRARIES)
