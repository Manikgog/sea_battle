#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "messenger_backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

           // Регистрируем QML тип
    qmlRegisterType<MessengerBackend>("MessengerApp", 1, 0, "MessengerBackend");

    QQmlApplicationEngine engine;

           // Создаем экземпляр бэкенда и передаем в QML
    MessengerBackend backend;
    engine.rootContext()->setContextProperty("backend", &backend);

    const QUrl url("qrc:/qml/Main.qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
