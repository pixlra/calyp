#
# CMakeLists for playuver lib tests component
#

INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

ADD_EXECUTABLE(PlaYUVerFrameQualityTests PlaYUVerFrameQualityTests.cpp)
TARGET_LINK_LIBRARIES(PlaYUVerFrameQualityTests ${PROJECT_LIBRARY} gtest_main)

INCLUDE(GoogleTest)
GTEST_DISCOVER_TESTS(PlaYUVerFrameQualityTests)
