#include "rea.h"
#include "awsStorage.h"
#include <QQmlApplicationEngine>

static rea2::regPip<QQmlApplicationEngine*> main_qml([](rea2::stream<QQmlApplicationEngine*>* aInput){
    static awsStorage storage("aws0");
    storage.initialize();
    aInput->out();
},  QJsonObject(), "loadMain");
