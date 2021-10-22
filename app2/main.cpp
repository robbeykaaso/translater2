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
    std::cout << rea2::getDefaultPipelineName().toStdString() << std::endl;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    rea2::pipeline::instance()->run<QQmlApplicationEngine*>("initRea", &engine, "", std::make_shared<rea2::scopeCache>(rea2::Json("rea-qml", rea2::Json("param", rea2::Json("use", false)))));

    auto ret = app.exec();

    return ret;
}
