#include "rea.h"

static rea::regPip<QString> create_menu_handler([](rea::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_menu___handler");
}, QJsonObject(), "create_handler");
