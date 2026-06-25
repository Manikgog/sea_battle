#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "local/model_adapter_local.hpp"
#include "network/model_adapter_network.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon/sea_battle.ico"));

    // Регистрируем типы вручную
    qmlRegisterType<ModelAdapterLocal>("sea_battle_combined", 1, 0, "ModelAdapterLocal");
    qmlRegisterType<ModelAdapterNetwork>("sea_battle_combined", 1, 0, "ModelAdapterNetwork");

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("sea_battle_combined", "Main");

    return app.exec();
}
