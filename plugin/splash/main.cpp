#include <QQmlApplicationEngine>
#include "rea.h"

static rea2::regPip<QQmlApplicationEngine*> splash_qml([](rea2::stream<QQmlApplicationEngine*>* aInput){
    //auto pth = QApplication::applicationDirPath() + "/main.qml";
    auto pth = "file:gui/Splash.qml";
    //QStringLiteral();
    aInput->data()->load(pth);
    aInput->out();
},  rea2::Json("before", "loadMain"));
