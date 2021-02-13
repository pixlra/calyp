# ####################################################################################################################################################
# DLLs and Win32 specific code
# ####################################################################################################################################################

FOREACH(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  STRING(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ".")
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ".")
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ".")
ENDFOREACH(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

SET(MSVC_DLL_DIR
    "MSVC_DLL_DIR"
    CACHE PATH "Where to find MSVC dlls")
# INSTALL(FILES ${MSVC_DLL_DIR}/msvcr120.dll DESTINATION bin ) INSTALL(FILES ${MSVC_DLL_DIR}/msvcp120.dll DESTINATION bin )

SET(QT_DIR
    "QT-Dir"
    CACHE PATH "Where to find QT Lib on Windows")
SET(QT_ICU_VERSION
    ""
    CACHE STRING "Qt ICU version of dll's")
SET(QT_DLL_POSTFIX
    ""
    CACHE STRING "Postfix to dll's name for debug")
SET(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} ${QT_DIR}")

SET(WINDLLIB
    "Dl-location"
    CACHE PATH "Where to find DL Lib on Windows")

IF(NOT USE_QT4)
  INSTALL(FILES ${QT_DIR}/bin/Qt5Core${QT_DLL_POSTFIX}.dll DESTINATION bin)
  INSTALL(FILES ${QT_DIR}/bin/Qt5Gui${QT_DLL_POSTFIX}.dll DESTINATION bin)
  INSTALL(FILES ${QT_DIR}/bin/Qt5Widgets${QT_DLL_POSTFIX}.dll DESTINATION bin)
  INSTALL(FILES ${QT_DIR}/bin/Qt5PrintSupport${QT_DLL_POSTFIX}.dll DESTINATION bin)
  INSTALL(FILES ${QT_DIR}/plugins/platforms/qwindows${QT_DLL_POSTFIX}.dll DESTINATION bin/platforms)
  INSTALL(FILES ${QT_DIR}/plugins/platforms/qminimal${QT_DLL_POSTFIX}.dll DESTINATION bin/platforms)
  INSTALL(FILES ${QT_DIR}/plugins/platforms/qoffscreen${QT_DLL_POSTFIX}.dll DESTINATION bin/platforms)

  IF(USE_FERVOR)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Network${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5WebKit${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5WebKitWidgets${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Quick${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Qml${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Multimedia${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Sql${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Positioning${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5Sensors${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5MultimediaWidgets${QT_DLL_POSTFIX}.dll DESTINATION bin)
    INSTALL(FILES ${QT_DIR}/bin/Qt5OpenGL${QT_DLL_POSTFIX}.dll DESTINATION bin)
  ENDIF()
ENDIF()

IF(USE_OPENCV)
  SET(OpenCV_DLL_DIR
      "${OpenCV_DIR}/bin"
      CACHE PATH "Where to find OpenCV dlls")
  SET(OpenCV_INSTALL_MODULES
      ${OPENCV_MODULES}
      CACHE STRING "List of OpenCV modules")
  FOREACH(opencv_module IN LISTS OpenCV_INSTALL_MODULES)
    INSTALL(FILES ${OpenCV_DLL_DIR}/opencv_${opencv_module}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll DESTINATION bin)
  ENDFOREACH()
ENDIF()

IF(USE_FFMPEG)
  SET(FFMPEG_DIR
      "FFmpeg-Dir"
      CACHE PATH "Where to find FFmpeg Lib on Windows")
  INSTALL(FILES ${FFMPEG_DIR}/bin/avcodec-${AVCODEC_VERSION}.dll DESTINATION bin)
  INSTALL(FILES ${FFMPEG_DIR}/bin/avformat-${AVFORMAT_VERSION}.dll DESTINATION bin)
  INSTALL(FILES ${FFMPEG_DIR}/bin/avutil-${AVUTIL_VERSION}.dll DESTINATION bin)
  INSTALL(FILES ${FFMPEG_DIR}/bin/swscale-${SWSCALE_VERSION}.dll DESTINATION bin)
  INSTALL(FILES ${FFMPEG_DIR}/bin/swresample-${SWRESAMPLE_VERSION}.dll DESTINATION bin)
ENDIF()
