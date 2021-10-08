#include "rea.h"

class resourceHandler{
public:
    resourceHandler(){
        rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            aInput->out();
        }, rea::Json("name", "copyToClipboard", "external", "qml"));
    }
};

static rea::regPip<QString> create_resource_handler([](rea::stream<QString>* aInput){
    static resourceHandler res_hdl;
    aInput->outs(aInput->data(), "create_resource_handler");
}, QJsonObject(), "create_handler");
