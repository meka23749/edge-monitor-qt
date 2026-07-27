cmake_minimum_required(VERSION 3.19)
project(EdgeMonitorQt LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(EdgeMonitorQt
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_translations(
    TARGETS EdgeMonitorQt
    TS_FILES EdgeMonitorQt_en_150.ts
)

target_link_libraries(EdgeMonitorQt
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS EdgeMonitorQt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET EdgeMonitorQt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
