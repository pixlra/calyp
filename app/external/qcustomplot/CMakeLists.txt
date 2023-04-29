#
# CMakeLists for qcustomplot
#

IF(${USE_QT6})
  FIND_PACKAGE(Qt6 QUIET COMPONENTS Core Gui Widgets PrintSupport)
  IF (NOT Qt6_FOUND)
    FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
  ENDIF()
ELSE()
  FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
ENDIF()


SET(qcustomplot_SRCS qcustomplot.cpp qcustomplot.h)

SET(CMAKE_AUTOMOC ON)

IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-deprecated-declarations -Wno-deprecated-enum-enum-conversion")
ENDIF()

ADD_LIBRARY(QCustomPlotLib STATIC ${qcustomplot_SRCS})
TARGET_LINK_LIBRARIES(QCustomPlotLib Qt::Core Qt::Gui Qt::Widgets Qt::PrintSupport)
TARGET_INCLUDE_DIRECTORIES(QCustomPlotLib PUBLIC .)
