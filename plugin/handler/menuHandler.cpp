#include "rea.h"

static rea2::regPip<QString> create_menu_handler([](rea2::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_menu___handler");
}, QJsonObject(), "create_handler");
