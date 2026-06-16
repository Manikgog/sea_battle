cmake_minimum_required(VERSION 3.16)

project(sea_battle_network VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appsea_battle
    main.cpp
    model_adapter.cpp
    model.cpp
    network_manager.cpp
    server.cpp
    client.cpp
)

qt_add_qml_module(appsea_battle
    URI sea_battle
    VERSION 1.0
    QML_FILES
    Main.qml
    SOURCES
    cell.hpp
    ship.hpp
    model.hpp
    utils.hpp
    model_adapter.hpp
    network_manager.hpp
    server.hpp
    client.hpp
    RESOURCES resources.qrc
)

set_target_properties(appsea_battle PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appsea_battle
    PRIVATE Qt6::Quick Qt6::Core Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS appsea_battle
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
