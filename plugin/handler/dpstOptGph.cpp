#include "rea.h"

static rea2::regPip<QString> create_dpst_optgph_handler([](rea2::stream<QString>* aInput){

    rea2::pipeline::instance()->add<QJsonObject>([](rea2::stream<QJsonObject>* aInput){
        qDebug() << aInput->data();
        aInput->out();
    }, rea2::Json("name", "client_getOperatorGraph", "befored", "getOperatorGraph", "external", "client"));

    aInput->outs(aInput->data(), "create_dpst_optgph___handler");
}, QJsonObject(), "create_handler");
