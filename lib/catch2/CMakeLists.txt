#
# CMakeLists for Calyp lis tests using catch2
#

# ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

SET(Calyp_Lib_Catch2_Tests_SRCS CalypFrameTests.cpp)

ADD_EXECUTABLE(CalypLibCatch2Tests ${Calyp_Lib_Catch2_Tests_SRCS})
TARGET_LINK_LIBRARIES(CalypLibCatch2Tests ${PROJECT_LIBRARY} Catch2::Catch2)

INCLUDE(CTest)
CATCH_DISCOVER_TESTS(CalypLibCatch2Tests)
