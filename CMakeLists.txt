cmake_minimum_required(VERSION 3.16)

project(EdgeMonitorQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

add_executable(EdgeMonitorQt
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(EdgeMonitorQt PRIVATE
    Qt6::Widgets
    Qt6::Charts
)