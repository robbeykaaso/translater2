#include "rea.h"

static rea::regPip<QString> create_status_handler([](rea::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_status___handler");
}, QJsonObject(), "create_handler");
