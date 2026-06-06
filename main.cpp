#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "model.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("sea_battle", "Main");

    Model model;
    Point p = model.getPoint(11);
    // for(int i = 1; i <= 100; ++i) {
    //     Point p = model.getPoint(i);
    // }

    return app.exec();
}
