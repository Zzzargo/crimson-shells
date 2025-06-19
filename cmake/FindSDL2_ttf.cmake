# Locate SDL2_ttf library
# This module defines
# SDL2_TTF_LIBRARY, the name of the library to link against
# SDL2_TTF_FOUND, if false, do not try to link to SDL2_ttf
# SDL2_TTF_INCLUDE_DIRS, where to find SDL_ttf.h
#
# This module responds to the the flag:
# SDL2_TTF_BUILDING_LIBRARY
# If this is defined, then no SDL2_TTF_main will be linked in because
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the proper link flags
# as part of the returned SDL2_TTF_LIBRARY variable.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(SDL2_TTF_INCLUDE_DIRS SDL_ttf.h
  HINTS
  $ENV{SDL2TTFDIR}
  PATH_SUFFIXES include/SDL2 include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/SDL2
  /usr/include/SDL2
  /sw/include/SDL2
  /opt/local/include/SDL2
  /opt/csw/include/SDL2
  /opt/include/SDL2
)

FIND_LIBRARY(SDL2_TTF_LIBRARIES
  NAMES SDL2_ttf
  HINTS
  $ENV{SDL2TTFDIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_ttf REQUIRED_VARS SDL2_TTF_LIBRARIES SDL2_TTF_INCLUDE_DIRS)