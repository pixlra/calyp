# General function to add a module It receives the name of the module and the filename where it is defined. Used in modules/CMakeLists.txt
MACRO(ADD_MODULE name file)
  SET(__modulename "Module${name}")
  OPTION(MODULE_${name} "Use module ${name}" ON)
  ADD_FEATURE_INFO(${name} MODULE_${name} "Module ${name}")
  IF(MODULE_${name})
    LIST(APPEND Calyp_Mod_SRCS ${MODULE_LOCATION}${file}.cpp)
    LIST(APPEND MODULES_LIST_NAME ${name})
    LIST(APPEND MODULES_LIST_FILES ${file})
    SET(Calyp_Mod_SRCS
        "${Calyp_Mod_SRCS}"
        PARENT_SCOPE
    )
    SET(MODULES_LIST_NAME
        "${MODULES_LIST_NAME}"
        PARENT_SCOPE
    )
    SET(MODULES_LIST_FILES
        "${MODULES_LIST_FILES}"
        PARENT_SCOPE
    )
  ENDIF()
  UNSET(__modulename)
ENDMACRO()

# General function to add a module It receives the name of the module and the filename where it is defined. Moreover it receives a VERSION and MODULES
# (OpenCV modules required) Used in modules/CMakeLists.txt
MACRO(ADD_MODULE_USE_OPENCV name file)
  IF(USE_OPENCV)
    SET(ADD_MODULE_USE_OPENCV_VERSION "2.4")
    CMAKE_PARSE_ARGUMENTS(ADD_MODULE_USE_OPENCV "" "VERSION" "MODULES" ${ARGN})
    IF(NOT (${OpenCV_VERSION} VERSION_LESS ${ADD_MODULE_USE_OPENCV_VERSION}))
      SET(_HAS_MODULES TRUE)
      FOREACH(opencv_module IN LISTS ADD_MODULE_USE_OPENCV_MODULES)
        STRING(TOUPPER "${opencv_module}" opencv_moduleUP)
        FIND_PACKAGE(OpenCV QUIET COMPONENTS ${opencv_module})
        IF(NOT ${OPENCV_${opencv_moduleUP}_FOUND} EQUAL "1")
          SET(_HAS_MODULES FALSE)
        ELSE()
          LIST(APPEND OPENCV_MODULES ${opencv_module})
        ENDIF()
      ENDFOREACH()
      IF(_HAS_MODULES)
        ADD_MODULE(${name} ${file})
      ENDIF()
      UNSET(OPENCV_${opencv_moduleUP}_FOUND)
      UNSET(_HAS_MODULES)
    ENDIF()
    UNSET(ADD_MODULE_USE_OPENCV_VERSION)
  ENDIF()
ENDMACRO()

# Create header
MACRO(CREATE_MODULE_MACROS)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ModulesListHeader.h "// This files add the header files of each module\n"
                                                             "#ifndef __MODULESLISTHEADER_H__\n#define __MODULESLISTHEADER_H__\n"
  )
  FOREACH(module ${MODULES_LIST_FILES})
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/ModulesListHeader.h "#include \"${module}.h\"\n")
  ENDFOREACH(module ${MODULES_LIST_FILES})

  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/ModulesListHeader.h "#define REGISTER_ALL_MODULES \\\n")
  FOREACH(module ${MODULES_LIST_NAME})
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/ModulesListHeader.h "Register( \"${module}\", &(${module}::Create) ); \\\n")
  ENDFOREACH(module ${MODULES_LIST_NAME})

  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/ModulesListHeader.h "\n#endif // __MODULESLISTHEADER_H__\n")
ENDMACRO()
