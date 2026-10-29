cmake_minimum_required(VERSION 3.21)
project(FileHunter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(FileHunter
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/searchworker.cpp
    src/searchworker.h
)

target_link_libraries(FileHunter PRIVATE Qt6::Widgets)

if(WIN32)
    set_target_properties(FileHunter PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS FileHunter
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
