cmake_minimum_required(VERSION 3.5)

project(JsonChartPlot VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")

find_package(Qt5 REQUIRED COMPONENTS Widgets PrintSupport Network)

set(PROJECT_SOURCES
        main.cpp
        chart.cpp
        mainwindow.cpp
        highlighter.cpp
        qcustomplot.cpp
        tcpserver.cpp
        mdichild.cpp
        expression.cpp
        table.cpp
)

add_executable(JsonChartPlot ${PROJECT_SOURCES})

target_link_libraries(JsonChartPlot PRIVATE Qt5::Widgets Qt5::PrintSupport Qt5::Network)

set_target_properties(JsonChartPlot PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(JsonChartPlot)
endif()
