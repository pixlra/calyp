#
# CMakeLists for modules_example
#
INCLUDE_DIRECTORIES(../../lib)

ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

ADD_EXECUTABLE(testStreamReading testStreamReading.cpp)
TARGET_LINK_LIBRARIES(testStreamReading ${PROJECT_LIBRARY})
