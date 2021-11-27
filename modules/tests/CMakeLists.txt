#
# CMakeLists for playuver lib tests component
#
OPTION(BUILD_MODULES_TESTS "Build unit tests of modules" ON)
IF(BUILD_MODULES_TESTS)

  FETCHCONTENT_MAKEAVAILABLE(googletest Catch2)

  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

  ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

  FOREACH(module ${MODULES_LIST_NAME})
    ADD_EXECUTABLE(TestModuleCreate_${module} TestModuleCreate.cpp)
    TARGET_COMPILE_DEFINITIONS(TestModuleCreate_${module} PUBLIC -DCALYP_TEST_MODULE_NAME=\"${module}\")
    TARGET_LINK_LIBRARIES(TestModuleCreate_${module} ${PROJECT_LIBRARY} CalypModules gtest gtest_main)
    ADD_TEST(TestModuleCreate_${module} TestModuleCreate_${module})

    ADD_EXECUTABLE(TestModuleProcessNumberOfFrames_${module} TestModuleProcessNumberOfFrames.cpp)
    TARGET_COMPILE_DEFINITIONS(TestModuleProcessNumberOfFrames_${module} PUBLIC -DCALYP_TEST_MODULE_NAME=\"${module}\")
    TARGET_LINK_LIBRARIES(TestModuleProcessNumberOfFrames_${module} ${PROJECT_LIBRARY} CalypModules gtest gtest_main)
    ADD_TEST(TestModuleProcessNumberOfFrames_${module} TestModuleProcessNumberOfFrames_${module})

  ENDFOREACH(module ${MODULES_LIST_NAME})
ENDIF()
