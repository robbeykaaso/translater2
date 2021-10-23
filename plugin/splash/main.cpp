#include <QQmlApplicationEngine>
#include "rea.h"

static rea2::regPip<QQmlApplicationEngine*> splash_qml([](rea2::stream<QQmlApplicationEngine*>* aInput){
    if (aInput->scope()->data<bool>("splash")){
        //auto pth = QApplication::applicationDirPath() + "/main.qml";
        auto pth = "file:nwlan_ui/gui/Splash.qml";
        //QStringLiteral();
        aInput->data()->load(pth);
    }
    aInput->out();
},  rea2::Json("before", "loadMain"));
