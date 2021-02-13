###
### CMakeLists for playuver lib tests component
###

INCLUDE_DIRECTORIES( ${GTEST_INCLUDE_DIRS} )

ADD_DEFINITIONS(-DCALYP_TEST_DATA_DIR=\"${CALYP_TEST_DATA_DIR}\")

foreach(module ${MODULES_LIST_NAME})
    ADD_EXECUTABLE(TestModuleCreate_${module} TestModuleCreate.cpp )
    TARGET_COMPILE_DEFINITIONS(TestModuleCreate_${module} PUBLIC -DCALYP_TEST_MODULE_NAME=\"${module}\")
    TARGET_LINK_LIBRARIES(TestModuleCreate_${module} ${PROJECT_LIBRARY} CalypModules gtest gtest_main )
    ADD_TEST(TestModuleCreate_${module} TestModuleCreate_${module})
endforeach(module ${MODULES_LIST_NAME})




