cmake_minimum_required(VERSION 3.16)

project(sea_battle_combined VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appsea_battle_combined
    main.cpp
    core/model.cpp
    local/bot.cpp
    local/model_adapter_local.cpp
    network/network_manager.cpp
    network/server.cpp
    network/client.cpp
    network/model_adapter_network.cpp
)

qt_add_qml_module(appsea_battle_combined
    URI sea_battle_combined
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/LocalGame.qml
        qml/NetworkGame.qml
)

# Добавляем заголовочные файлы отдельно, без QML_ELEMENT
target_sources(appsea_battle_combined
    PRIVATE
        core/cell.hpp
        core/ship.hpp
        core/model.hpp
        core/utils.hpp
        local/bot.hpp
        local/model_adapter_local.hpp
        network/network_manager.hpp
        network/server.hpp
        network/client.hpp
        network/model_adapter_network.hpp
)

set_target_properties(appsea_battle_combined PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appsea_battle_combined
    PRIVATE Qt6::Quick Qt6::Core Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS appsea_battle_combined
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
