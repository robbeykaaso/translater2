#include "rea.h"

class resourceHandler{
public:
    resourceHandler(){
        rea2::pipeline::instance()->add<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea2::Json("name", "copyToClipboard", "external", "qml"));
    }
};

static rea2::regPip<QString> create_resource_handler([](rea2::stream<QString>* aInput){
    static resourceHandler res_hdl;
    aInput->outs(aInput->data(), "create_resource___handler");
}, QJsonObject(), "create_handler");
