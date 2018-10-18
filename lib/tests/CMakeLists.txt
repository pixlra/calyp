###
### CMakeLists for playuver lib tests component
###

INCLUDE_DIRECTORIES( ${GTEST_INCLUDE_DIRS} )

ADD_EXECUTABLE(PlaYUVerFrameQualityTests PlaYUVerFrameQualityTests.cpp )
TARGET_LINK_LIBRARIES(PlaYUVerFrameQualityTests ${PROJECT_LIBRARY} ${PlaYUVerLib_DEPS} gtest gtest_main )
ADD_TEST(PlaYUVerFrameQualityTests PlaYUVerFrameQualityTests)

