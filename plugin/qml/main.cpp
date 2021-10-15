#include <QQmlApplicationEngine>
#include "rea.h"

static rea::regPip<QQmlApplicationEngine*> main_qml([](rea::stream<QQmlApplicationEngine*>* aInput){
    aInput->data()->load("file:gui/html_main.qml");
    aInput->out();
},  rea::Json("name", "loadMain"));
