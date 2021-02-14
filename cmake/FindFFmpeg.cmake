# * Try to find the required ffmpeg components(default: AVFORMAT, AVUTIL, AVCODEC)
#
# Once done this will define FFMPEG_FOUND         - System has the all required components. FFMPEG_INCLUDE_DIRS  - Include directory necessary for
# using the required components headers. FFMPEG_LIBRARIES     - Link these to use the required ffmpeg components. FFMPEG_DEFINITIONS   - Compiler
# switches required for using the required ffmpeg components.
#
# For each of the components it will additionally set. - AVCODEC - AVDEVICE - AVFORMAT - AVUTIL - POSTPROCESS - SWSCALE the following variables will
# be defined <component>_FOUND        - System has <component> <component>_INCLUDE_DIRS - Include directory necessary for using the <component>
# headers <component>_LIBRARIES    - Link these to use <component> <component>_DEFINITIONS  - Compiler switches required for using <component>
# <component>_VERSION      - The components version
#
# Copyright (c) 2006, Matthias Kretz, <kretz@kde.org> Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org> Copyright (c) 2011, Michael Jansen,
# <kde@michael-jansen.biz>
#
# Redistribution and use is allowed according to the terms of the BSD license. For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE(FindPackageHandleStandardArgs)

# The default components were taken from a survey over other FindFFMPEG.cmake files
IF(NOT FFmpeg_FIND_COMPONENTS)
  SET(FFmpeg_FIND_COMPONENTS AVCODEC AVFORMAT AVUTIL)
ENDIF()

#
# Macro: set_component_found
#
# Marks the given component as found if both *_LIBRARIES AND *_INCLUDE_DIRS is present.
#
MACRO(set_component_found _component)
  IF(${_component}_LIBRARIES AND ${_component}_INCLUDE_DIRS)
    # message(STATUS "  - ${_component} found.")
    SET(${_component}_FOUND TRUE)
  ELSE()
    # message(STATUS "  - ${_component} not found.")
  ENDIF()
ENDMACRO()

#
# Macro: find_component
#
# Checks for the given component by invoking pkgconfig and then looking up the libraries and include directories.
#
MACRO(find_component _component _pkgconfig _library _header)

  IF(NOT WIN32)
    # use pkg-config to get the directories and then use these values in the FIND_PATH() and FIND_LIBRARY() calls
    FIND_PACKAGE(PkgConfig)
    IF(PKG_CONFIG_FOUND)
      PKG_CHECK_MODULES(PC_${_component} ${_pkgconfig})
    ENDIF()
  ENDIF(NOT WIN32)

  FIND_PATH(
    ${_component}_INCLUDE_DIRS ${_header}
    HINTS ${PC_LIB${_component}_INCLUDEDIR} ${PC_LIB${_component}_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
  )

  FIND_LIBRARY(
    ${_component}_LIBRARIES
    NAMES ${_library}
    HINTS ${PC_LIB${_component}_LIBDIR} ${PC_LIB${_component}_LIBRARY_DIRS}
  )

  SET(${_component}_DEFINITIONS
      ${PC_${_component}_CFLAGS_OTHER}
      CACHE STRING "The ${_component} CFLAGS."
  )
  SET(${_component}_VERSION
      ${PC_${_component}_VERSION}
      CACHE STRING "The ${_component} version number."
  )

  SET_COMPONENT_FOUND(${_component})

  MARK_AS_ADVANCED(${_component}_INCLUDE_DIRS ${_component}_LIBRARIES ${_component}_DEFINITIONS ${_component}_VERSION)

ENDMACRO()

# Check for cached results. If there are skip the costly part.
IF(NOT FFMPEG_LIBRARIES)

  # Check for all possible component.
  FIND_COMPONENT(AVCODEC libavcodec avcodec libavcodec/avcodec.h)
  FIND_COMPONENT(AVFORMAT libavformat avformat libavformat/avformat.h)
  FIND_COMPONENT(AVDEVICE libavdevice avdevice libavdevice/avdevice.h)
  FIND_COMPONENT(AVUTIL libavutil avutil libavutil/avutil.h)
  FIND_COMPONENT(SWSCALE libswscale swscale libswscale/swscale.h)
  FIND_COMPONENT(POSTPROC libpostproc postproc libpostproc/postprocess.h)

  # Check if the required components were found and add their stuff to the FFMPEG_* vars.
  FOREACH(_component ${FFmpeg_FIND_COMPONENTS})
    IF(${_component}_FOUND)
      # message(STATUS "Required component ${_component} present.")
      SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${${_component}_LIBRARIES})
      SET(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${${_component}_DEFINITIONS})
      LIST(APPEND FFMPEG_INCLUDE_DIRS ${${_component}_INCLUDE_DIRS})
    ELSE()
      # message(STATUS "Required component ${_component} missing.")
    ENDIF()
  ENDFOREACH()

  # Build the include path with duplicates removed.
  IF(FFMPEG_INCLUDE_DIRS)
    LIST(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)
  ENDIF()

  # cache the vars.
  SET(FFMPEG_INCLUDE_DIRS
      ${FFMPEG_INCLUDE_DIRS}
      CACHE STRING "The FFmpeg include directories." FORCE
  )
  SET(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      CACHE STRING "The FFmpeg libraries." FORCE
  )
  SET(FFMPEG_DEFINITIONS
      ${FFMPEG_DEFINITIONS}
      CACHE STRING "The FFmpeg cflags." FORCE
  )

  MARK_AS_ADVANCED(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES FFMPEG_DEFINITIONS)

ENDIF()

# Now set the noncached _FOUND vars for the components.
FOREACH(_component AVCODEC AVDEVICE AVFORMAT AVUTIL POSTPROCESS SWSCALE)
  SET_COMPONENT_FOUND(${_component})
ENDFOREACH()

# Compile the list of required vars
SET(_FFmpeg_REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS)
FOREACH(_component ${FFmpeg_FIND_COMPONENTS})
  LIST(APPEND _FFmpeg_REQUIRED_VARS ${_component}_LIBRARIES ${_component}_INCLUDE_DIRS)
ENDFOREACH()

# Give a nice error message if some of the required vars are missing.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFmpeg DEFAULT_MSG ${_FFmpeg_REQUIRED_VARS})
