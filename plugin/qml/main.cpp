#include <QQmlApplicationEngine>
#include "rea.h"

static rea2::regPip<QQmlApplicationEngine*> main_qml([](rea2::stream<QQmlApplicationEngine*>* aInput){
    aInput->data()->load("file:gui/html_main.qml");
    aInput->out();
},  rea2::Json("name", "loadMain"));
