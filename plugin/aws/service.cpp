#include "rea.h"
#include "awsStorage.h"
#include <QQmlApplicationEngine>

static rea::regPip<QQmlApplicationEngine*> main_qml([](rea::stream<QQmlApplicationEngine*>* aInput){
    static awsStorage storage("aws0");
    storage.initialize();
    aInput->out();
},  QJsonObject(), "loadMain");
