#
# CMakeLists for Calyp lis tests using catch2
#

FETCHCONTENT_MAKEAVAILABLE(Catch2)

SET(Calyp_App_Catch2_Tests_SRCS CalypAppTests.cpp CalypResourcesTests.cpp)

ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

ADD_EXECUTABLE(CalypAppCatch2Tests ${Calyp_App_Catch2_Tests_SRCS})
TARGET_LINK_LIBRARIES(
  CalypAppCatch2Tests
  ${PROJECT_LIBRARY}
  CalypModules
  ${APP_LINKER_LIBS}
  ${QT_LINKER_LIBS}
  QCustomPlotLib
  Catch2::Catch2
)
CATCH_DISCOVER_TESTS(CalypAppCatch2Tests)
