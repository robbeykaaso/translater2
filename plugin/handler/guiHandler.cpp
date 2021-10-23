#include "rea.h"

static rea2::regPip<QString> create_menu_handler([](rea2::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_menu___handler");
}, QJsonObject(), "create_handler");

static rea2::regPip<QString> create_status_handler([](rea2::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_status___handler");
}, QJsonObject(), "create_handler");

static rea2::regPip<QString> create_titlebar_handler([](rea2::stream<QString>* aInput){
    aInput->outs(aInput->data(), "create_titlebar___handler");
}, QJsonObject(), "create_handler");
