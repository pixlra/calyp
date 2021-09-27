#
# CMakeLists for qcustomplot
#

FIND_PACKAGE(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)

SET(qcustomplot_SRCS qcustomplot.cpp qcustomplot.h)

SET(CMAKE_AUTOMOC ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-deprecated-enum-enum-conversion")

ADD_LIBRARY(QCustomPlotLib STATIC ${qcustomplot_SRCS})
TARGET_LINK_LIBRARIES(QCustomPlotLib Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)
TARGET_INCLUDE_DIRECTORIES(QCustomPlotLib PUBLIC .)
