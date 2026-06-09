#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <iostream>
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

    bool placing_result = model.automaticShipsPlacing();

    int count_cells = 0;
    for(int i = 0; i < model.getPlayingField().size(); ++i) {
       if(i%10 == 0) {
           std::cout << std::endl;
       }
       if(model.getPlayingField()[i]._isOccupied) {
           count_cells++;
           std::cout << "+ ";
       } else if(model.getPlayingField()[i]._isAllowed == false) {
           std::cout << "  ";
       }
       else{
           std::cout << "  ";
       }

    }
   std::cout << std::endl;
   std::cout << std::endl;

    return app.exec();
}
