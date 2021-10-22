#include "rea.h"

static rea2::regPip<QString> create_status_handler([](rea2::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_status___handler");
}, QJsonObject(), "create_handler");
