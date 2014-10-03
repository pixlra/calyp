######################################################################################
# Install
######################################################################################

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin )
INSTALL(FILES ${RSC_DIR}/playuver.desktop DESTINATION share/applications )
INSTALL(FILES ${RSC_DIR}/playuver.png DESTINATION share/icons )
INSTALL(FILES ${RSC_DIR}/x-raw.xml DESTINATION share/mime/video )
INSTALL(FILES ${CMAKE_BINARY_DIR}/PlaYUVerConfig.cmake DESTINATION share/playuver )

IF( WIN32 )

  SET(MSVC_DLL_DIR "MSVC_DLL_DIR" CACHE PATH "Where to find MSVC dlls")
  INSTALL(FILES ${MSVC_DLL_DIR}/msvcr120.dll DESTINATION bin )
  INSTALL(FILES ${MSVC_DLL_DIR}/msvcp120.dll DESTINATION bin )
  
ENDIF()

IF( PLAYUVER_INSTALL_LIBS )
  FILE( GLOB PLAYUVER_LIB_INCLUDE_FILES   ${SRC_DIR}/lib/*.h ) 
  INSTALL(FILES ${PLAYUVER_LIB_INCLUDE_FILES} DESTINATION include )
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libPlaYUVerLib.a DESTINATION lib )
ENDIF()


######################################################################################
# Create deb package
######################################################################################

SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_CONTACT "Joao Carreira (jfmcarreira@gmail.com), Luis Lucas (luisfrlucas@gmail.com)")
SET(CPACK_PACKAGE_VERSION_MAJOR ${PLAYUVER_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${PLAYUVER_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${PLAYUVER_VERSION_PATH})
SET(CPACK_PACKAGE_VERSION ${PLAYUVER_VERSION})

SET(CPACK_PACKAGE_ARCHITECTURE "amd64")

IF( UNIX )

  SET(CPACK_GENERATOR "DEB;ZIP")
  SET(OS "Linux")

  SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "plaYUVer is an open-source QT based raw video player")
  SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Joao Carreira (jfmcarreira@gmail.com), Luis Lucas (luisfrlucas@gmail.com)")
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE} )
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS "")
  IF( USE_QT4 )
    LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4:4.8.5), libqtgui4 (>= 4:4.8.5)" )
  ELSE()
    LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5gui5, libqt5widgets5")
  ENDIF()
  IF( USE_OPENCV )
    LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS ",")
    LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libopencv-dev (>= 2.4.8)")
  ENDIF()
  IF( USE_FFMPEG )
    LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS ",")
    LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libavformat54, libavcodec54, libavutil52")
  ENDIF()
 
  STRING(REPLACE ";" "" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} )

  #EXECUTE_PROCESS(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE DAY)
  #string(STRIP ${DAY} DAY)

ENDIF()

IF( WIN32 )

  SET(CPACK_GENERATOR "ZIP")
  SET(OS "Windows")
  SET(GIT_BRANCH "devel") # temp work around
  #EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${DAY})
  #string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${DAY} ${${DAY}})
 
ENDIF()

IF( USE_QT4 )
  SET( APPEND_VERSION "${APPEND_VERSION}_wQT4" )
ELSE() 
  SET( APPEND_VERSION "${APPEND_VERSION}_wQT5" )
ENDIF()
IF( USE_FFMPEG )
  SET( APPEND_VERSION "${APPEND_VERSION}_wFFmpeg" )
ENDIF()
IF( USE_OPENCV )
  SET( APPEND_VERSION "${APPEND_VERSION}_wOpenCV" )
ENDIF()

IF( PLAYUVER_LONGVERSIONNAME )
  SET( CPACK_PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}${APPEND_VERSION}-${OS}-${CPACK_PACKAGE_ARCHITECTURE}" )
  SET( PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME} )
ELSE()
#  message(STATUS ${GIT_BRANCH} )
  STRING(REPLACE "master" "stable" __version_name_deb ${GIT_BRANCH}  )
  STRING(REPLACE "devel" "latest" __version_name_deb ${GIT_BRANCH} )
  message(STATUS ${GIT_BRANCH} )
  SET( CPACK_PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${__version_name_deb}${APPEND_VERSION}-${OS}-${CPACK_PACKAGE_ARCHITECTURE}" )  
  SET( PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME} )
ENDIF()

INCLUDE(CPack)
