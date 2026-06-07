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
    //Point p = model.getPoint(11);
    if(model.automaticShipsPlacing()) {
        std::cout << "размещение прошло успешно" << std::endl;
    }
    std::vector<Cell> playingField = model.getPlayingField();
    int cell_counter = 0;
    int row = 0;
    std::cout << "\t";
    for(int i = 0; i < 10; ++i) {
        std::cout << i << "\t";
    }
    std::cout << std::endl;

    for(int i = 0; i < playingField.size(); ++i) {
        if(i%10 == 0) {
            std::cout << std::endl;
            std::cout << i/10 << "\t";
        }
        if(playingField[i]._isOccupied) {
            cell_counter++;
            std::cout << "+\t";
        }else {
            std::cout << "0\t";
        }
    }
    std::cout << std::endl;
    qDebug() << "cell_counter =" << cell_counter;

    return app.exec();
}
