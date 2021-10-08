#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QApplication>
#include <iostream>
#include <QWindow>
#include "rea.h"
#include "server.h"


#include <QFile>
#include <QJsonDocument>
int main(int argc, char *argv[])
{
    std::cout << rea::getDefaultPipelineName().toStdString() << std::endl;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    rea::pipeline::instance()->run<QQmlApplicationEngine*>("initRea", &engine, "", std::make_shared<rea::scopeCache>(rea::Json("rea-qml", rea::Json("param", rea::Json("use", false)))));

    auto ret = app.exec();

    return ret;
}
