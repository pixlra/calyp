######################################################################################
# Create deb package
######################################################################################

SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(PACKAGE_NAME "" CACHE STRING "Tag instead of version" )

SET(CPACK_PACKAGE_CONTACT "Joao Carreira (jfmcarreira@gmail.com), Luis Lucas (luisfrlucas@gmail.com)")
SET(CPACK_PACKAGE_VERSION ${CALYP_VERSION})
SET(CPACK_PACKAGE_ARCHITECTURE "amd64")

IF( USE_QT4 )
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4:4.8.5), libqtgui4 (>= 4:4.8.5), libqt4-dbus (>= 4:4.8.5)" )
  SET( APPEND_VERSION "${APPEND_VERSION}_wQT4" )
ELSE()
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a (>= 5.2.0), libqt5dbus5 (>= 5.0.2), libqt5gui5 (>= 5.0.2) | libqt5gui5-gles (>= 5.0.2), libqt5printsupport5 (>= 5.0.2), libqt5widgets5 (>= 5.2.0)")
  SET( APPEND_VERSION "${APPEND_VERSION}_wQT5" )
ENDIF()

IF( USE_OPENCV )
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS ",")
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libopencv-core2.4, libopencv-highgui2.4")
  SET( APPEND_VERSION "${APPEND_VERSION}_wOpenCV" )
ENDIF()

IF( USE_FFMPEG )
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS ",")
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libavcodec54 (>= 7:1.2.5~) | libavcodec-extra-54 (>= 7:1.2.5~), libavformat54 (>= 7:1.2.5~), libavutil52 (>= 7:1.2.5~)")
  SET( APPEND_VERSION "${APPEND_VERSION}_wFFmpeg" )
ENDIF()

IF( UNIX )
  SET(CPACK_GENERATOR "DEB;ZIP")
  SET(OS "Linux")
  SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Calyp is an open-source QT based raw video player")
  SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Joao Carreira (jfmcarreira@gmail.com), Luis Lucas (luisfrlucas@gmail.com)")
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE} )
  LIST(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS ", libc6 (>= 2.14), libgcc1 (>= 1:4.1.1) , libstdc++6 (>= 4.1.1)")
  STRING(REPLACE ";" "" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} )
ENDIF()
IF( WIN32 )
  SET(CPACK_GENERATOR "ZIP;NSIS")
  SET(OS "Windows")
  
  set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})
  set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
  set(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})
  
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "calyp" )
  
  SET(GIT_BRANCH "master") # temp work around
ENDIF()

IF( PACKAGE_NAME STREQUAL "")
  SET( PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}${APPEND_VERSION}-${OS}-${CPACK_PACKAGE_ARCHITECTURE}" )
ELSE()
  SET( PACKAGE_FILE_NAME  "${CPACK_PACKAGE_NAME}-${PACKAGE_NAME}${APPEND_VERSION}-${OS}-${CPACK_PACKAGE_ARCHITECTURE}" )
ENDIF()
SET( CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME} )

INCLUDE(CPack)

MESSAGE( STATUS "    Package Name:"        "${PACKAGE_FILE_NAME}"  )
