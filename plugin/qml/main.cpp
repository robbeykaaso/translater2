#include <QQmlApplicationEngine>
#include "rea.h"

static rea::regPip<QQmlApplicationEngine*> main_qml([](rea::stream<QQmlApplicationEngine*>* aInput){
    //auto pth = QApplication::applicationDirPath() + "/main.qml";
    auto pth = "file:service/Splash.qml";
    //QStringLiteral();
    aInput->data()->load(pth);
    aInput->data()->load("file:gui/html_main.qml");
    aInput->out();
},  rea::Json("name", "loadMain"));
