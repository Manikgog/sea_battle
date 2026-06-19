cmake_minimum_required(VERSION 3.16)

project(sea_battle_network_version VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network WebSockets)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appsea_battle_network_version
    main.cpp
    messenger_backend.hpp
    messenger_backend.cpp
    network_manager.hpp
    network_manager.cpp
    model_adapter.hpp
    model_adapter.cpp
    model.hpp
    ship.hpp
    cell.hpp
    model.cpp
    utils.hpp


)

qt_add_qml_module(appsea_battle_network_version
    URI sea_battle_network_version
    VERSION 1.0
    QML_FILES
        Main.qml
        MessegeDelegate.qml
        SOURCES model_adapter.hpp
        SOURCES model.hpp
        SOURCES ship.hpp
        SOURCES cell.hpp
        SOURCES model.cpp
        SOURCES utils.hpp
        SOURCES model_adapter.cpp
)

qt_add_resources(appsea_battle_network_version "qml_resources"
    PREFIX "/qml"
    FILES
        Main.qml
        MessegeDelegate.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appsea_battle_network_version PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appsea_battle_network_version
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appsea_battle_network_version
    PRIVATE Qt6::Quick Qt6::Core Qt6::Network Qt6::WebSockets
)

include(GNUInstallDirs)
install(TARGETS appsea_battle_network_version
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
