#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "model_adapter.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    qmlRegisterSingletonType<ModelAdapter>("sea_battle", 1, 0, "ModelAdapter",
                                           [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
                                               Q_UNUSED(engine)
                                               Q_UNUSED(scriptEngine)
                                               return new ModelAdapter();
                                           });

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("sea_battle", "Main");

    return app.exec();
}
