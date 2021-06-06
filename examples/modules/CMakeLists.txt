#
# CMakeLists for modules_example
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0)

PROJECT("modules_example")

# THIS FIND_PACKAGE( Calyp ) INCLUDE_DIRECTORIES( ${CALYP_INCLUDE_DIRS} ) OR
INCLUDE_DIRECTORIES(../../lib)

ADD_LIBRARY(FrameBinarization SHARED FrameBinarization_APIv1.cpp)
TARGET_LINK_LIBRARIES(FrameBinarization ${PROJECT_LIBRARY})

ADD_LIBRARY(AbsoluteFrameDifferenceExample SHARED AbsoluteFrameDifferenceExample.cpp)
TARGET_LINK_LIBRARIES(AbsoluteFrameDifferenceExample ${PROJECT_LIBRARY})
