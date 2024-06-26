#
# CMakeLists for calyp
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0)

# ####################################################################################################################################################
# Project Definition
# ####################################################################################################################################################

SET(PROJECT_NAME
    "calyp"
    CACHE INTERNAL "" FORCE
)
SET(PROJECT_LIBRARY
    "Calyp"
    CACHE INTERNAL "" FORCE
)

SET(CALYP_VERSION_MAJOR "2")
SET(CALYP_VERSION_MINOR "2")
SET(CALYP_VERSION_PATCH "0")
SET(CALYP_VERSION
    "${CALYP_VERSION_MAJOR}.${CALYP_VERSION_MINOR}.${CALYP_VERSION_PATCH}"
    CACHE INTERNAL "" FORCE
)

PROJECT(
  ${PROJECT_NAME}
  VERSION ${CALYP_VERSION}
  LANGUAGES "CXX"
)

# ####################################################################################################################################################
# CMake  functions
# ####################################################################################################################################################
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModulesFct.cmake)
INCLUDE(FeatureSummary)
INCLUDE(GNUInstallDirs)

# ####################################################################################################################################################
# Variables
# ####################################################################################################################################################

OPTION(BUILD_APP "Build GUI Application" ON)
OPTION(BUILD_TOOLS "Build Command Line Application" ON)
OPTION(BUILD_EXAMPLES "Build Examples" OFF)
OPTION(BUILD_DOC "Build Documentation" OFF)
OPTION(BUILD_TESTS "Build Tests" OFF)
OPTION(BUILD_WITH_SANITIZERS "Build with sanitizers" OFF)
OPTION(BUILD_WITH_THREAD_SANITIZER "Build with thread sanitizer" OFF)

OPTION(USE_FERVOR "Add Fervor support" OFF)

OPTION(USE_DYNLOAD "Use dynamic load of modules" ON)
OPTION(USE_SSE "Build with SSE support" ON)
OPTION(USE_WERROR "Warnings as errors" OFF)
OPTION(USE_STATIC "Use static libs" OFF)
OPTION(USE_IWYU "Use include what you use tool" OFF)

# ####################################################################################################################################################
# Release configuration
# ####################################################################################################################################################

# To be switched on when releasing.
OPTION(RELEASE_BUILD "Remove Git revision from program version (use for stable releases)" ON)

# Get current version.
SET(CALYP_VERSION_STRING "${CALYP_VERSION}")
IF(NOT RELEASE_BUILD AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  # Probably a Git workspace; determine the revision.
  FIND_PACKAGE(Git QUIET)
  IF(GIT_FOUND)
    EXEC_PROGRAM(
      ${GIT_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}
      ARGS "describe --tags"
      OUTPUT_VARIABLE CALYP_GIT_REVISION
      RETURN_VALUE TAG_RESULT
    )
    # git describe --tags failed; maybe the repository was checked with depth=1.
    IF(NOT ${TAG_RESULT} EQUAL 0)
      EXEC_PROGRAM(
        ${GIT_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}
        ARGS "describe --always"
        OUTPUT_VARIABLE CALYP_GIT_REVISION
      )
    ENDIF()
    MESSAGE(STATUS "Calyp Git revision: ${CALYP_GIT_REVISION}")
    SET(CALYP_VERSION_STRING "${CALYP_VERSION} (rev. ${CALYP_GIT_REVISION})")
  ELSE()
    MESSAGE(STATUS "Calyp Git revision could not be determined")
  ENDIF()
ENDIF()

SET(UPDATE_CHANNEL
    "stable"
    CACHE STRING "Update channel"
)
# CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/CalypUpdate.xml.in ${CMAKE_CURRENT_BINARY_DIR}/CalypUpdate-${UPDATE_CHANNEL}.xml )

# ####################################################################################################################################################
# Dependencies
# ####################################################################################################################################################
INCLUDE(FetchContent)

SET(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/deps")

# Doxygen
IF(BUILD_DOC)
  FIND_PACKAGE(Doxygen)
  SET(BUILD_DOC ${Doxygen_FOUND})
ENDIF()
SET_PACKAGE_PROPERTIES(
  Doxygen PROPERTIES
  URL "http://www.doxygen.org/"
  DESCRIPTION "Doxygen"
  TYPE OPTIONAL
)

# Catch2 and Google Tests
IF(BUILD_TESTS)
  FETCHCONTENT_DECLARE(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  SET(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE
  )
  FETCHCONTENT_DECLARE(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.2
  )

  SET_PACKAGE_PROPERTIES(
    googletest PROPERTIES
    URL "https://github.com/google/googletest"
    DESCRIPTION "Google unit tests"
    TYPE OPTIONAL
  )

  SET_PACKAGE_PROPERTIES(
    Catch2 PROPERTIES
    URL "https://github.com/catchorg/Catch2"
    DESCRIPTION "Catch2 unit tests"
    TYPE OPTIONAL
  )
  FETCHCONTENT_MAKEAVAILABLE(googletest Catch2)

  LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/deps/catch2-src/extras)
  INCLUDE(CTest)
  INCLUDE(Catch)
  ENABLE_TESTING()

  SET(CALYP_TEST_DATA_DIR
      "CALYP_TEST_DATA_DIR"
      CACHE PATH "Location of video information for testing"
  )
ENDIF()

# OpenCV
OPTION(USE_OPENCV "Add OpenCV support" ON)
IF(USE_OPENCV)

  FIND_PACKAGE(OpenCV COMPONENTS core highgui imgproc video)
  IF(${OpenCV_FOUND})
    SET(USE_OPENCV TRUE)
    LIST(APPEND OPENCV_MODULES core highgui imgproc video)
    FIND_PACKAGE(OpenCV COMPONENTS videoio photo)
    IF(${OPENCV_VIDEOIO_FOUND})
      LIST(APPEND OPENCV_MODULES videoio)
    ENDIF()
    IF(${OPENCV_PHOTO_FOUND})
      LIST(APPEND OPENCV_MODULES photo)
    ENDIF()
  ELSE()
    SET(USE_OPENCV FALSE)
  ENDIF()
ENDIF()
SET_PACKAGE_PROPERTIES(
  OpenCV PROPERTIES
  URL "http://opencv.willowgarage.com"
  DESCRIPTION "OpenCV library support in CalypFrame"
  TYPE OPTIONAL
)

# FFmpeg
OPTION(USE_FFMPEG "Add FFMPEG support" ON)
IF(USE_FFMPEG)
  FIND_PACKAGE(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE)
  SET(USE_FFMPEG ${FFMPEG_FOUND})
ENDIF()
SET_PACKAGE_PROPERTIES(
  FFmpeg PROPERTIES
  URL "http://ffmpeg.org/"
  DESCRIPTION "Libav library support in CalypStream"
  TYPE OPTIONAL
)

IF(WIN32)
  SET(USE_STATIC ON)
  INCLUDE(cmake/Win32.cmake)
ENDIF()

# ####################################################################################################################################################
# CMake Defin1itions
# ####################################################################################################################################################

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(FeatureSummary)

# must go before the project command
SET(CMAKE_CONFIGURATION_TYPES
    "Debug;Release"
    CACHE STRING "Configs" FORCE
)
IF(DEFINED CMAKE_BUILD_TYPE AND CMAKE_VERSION VERSION_GREATER "2.8")
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
ENDIF()
SET(CMAKE_CXX_FLAGS "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# ####################################################################################################################################################
# Compiler options
# ####################################################################################################################################################

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
IF(MSVC)
  ADD_COMPILE_OPTIONS(/std:c++20)
ELSE()
  IF(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
    ADD_DEFINITIONS(-std=gnu++2a)
  ELSE()
    ADD_DEFINITIONS(-std=c++20)
  ENDIF()
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "No build type selected, default to Release")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -funroll-loops")

# if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic") endif()

IF(USE_WERROR)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DCLP_DEBUG")
IF(BUILD_WITH_SANITIZERS)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
ENDIF()
IF(BUILD_WITH_THREAD_SANITIZER)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
ENDIF()

IF(USE_IWYU)
  FIND_PROGRAM(iwyu_path NAMES include-what-you-use iwyu REQUIRED)
  SET(CXX_INCLUDE_WHAT_YOU_USE ON)
ENDIF()

# ####################################################################################################################################################
# Configure targets
# ####################################################################################################################################################

ADD_FEATURE_INFO(CalypApp BUILD_APP "Build Graphical interface")
ADD_FEATURE_INFO(CalypTools BUILD_TOOLS "Build Command line tool")
ADD_FEATURE_INFO(DynLoad USE_DYNLOAD "Support for dynamic module load")
ADD_FEATURE_INFO(SSE USE_SSE "SSE instructions support")
ADD_FEATURE_INFO(WErrors USE_WERROR "Warnings as errors")

ADD_SUBDIRECTORY(lib)

IF(${BUILD_TOOLS} OR ${BUILD_APP})
  ADD_SUBDIRECTORY(modules)
ENDIF()

IF(${BUILD_TOOLS})
  ADD_SUBDIRECTORY(tools)
ENDIF()

IF(${BUILD_APP})
  ADD_SUBDIRECTORY(app)
ENDIF()

IF(${BUILD_EXAMPLES})
  ADD_SUBDIRECTORY(examples)
ENDIF()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# ####################################################################################################################################################
# Final information dump
# ####################################################################################################################################################
MESSAGE("")
MESSAGE(STATUS "${PROJECT_NAME} INFORMATION")
MESSAGE(STATUS "Version: " "${CALYP_VERSION_STRING}")
MESSAGE(STATUS "Configuration:")
MESSAGE(STATUS "    Static libs: " "${USE_STATIC}")
MESSAGE(STATUS "    Build type: " "${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "    Build flags: " "${CMAKE_CXX_FLAGS}")

FEATURE_SUMMARY(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
