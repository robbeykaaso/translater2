#include "rea.h"

static rea::regPip<QString> create_resource_handler([](rea::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_resource_handler");
}, QJsonObject(), "create_handler");
