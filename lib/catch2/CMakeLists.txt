#
# CMakeLists for Calyp lis tests using catch2
#

FETCHCONTENT_MAKEAVAILABLE(Catch2)


SET(Calyp_Lib_Catch2_Tests_SRCS
    CalypLibTests.cpp
    CalypStreamTests.cpp
    CalypFrameTests.cpp
)

ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

ADD_EXECUTABLE(CalypLibCatch2Tests ${Calyp_Lib_Catch2_Tests_SRCS})
TARGET_LINK_LIBRARIES(CalypLibCatch2Tests ${PROJECT_LIBRARY} Catch2::Catch2WithMain)
CATCH_DISCOVER_TESTS(CalypLibCatch2Tests)
