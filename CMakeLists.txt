cmake_minimum_required(VERSION 3.19)
project(Pet_catBug LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)
qt_add_executable(Pet_catBug
    main.cpp
    widget.cpp
    widget.h
    image.qrc
)
target_link_libraries(Pet_catBug
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS Pet_catBug
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Pet_catBug
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
