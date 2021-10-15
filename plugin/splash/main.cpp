#include <QQmlApplicationEngine>
#include "rea.h"

static rea::regPip<QQmlApplicationEngine*> splash_qml([](rea::stream<QQmlApplicationEngine*>* aInput){
    //auto pth = QApplication::applicationDirPath() + "/main.qml";
    auto pth = "file:gui/Splash.qml";
    //QStringLiteral();
    aInput->data()->load(pth);
    aInput->out();
},  rea::Json("before", "loadMain"));
