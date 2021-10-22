#include "rea.h"
#include "handler.h"
#include <QJsonDocument>
#include <QFileInfo>

class threeDHandler : private handler{
private:
    QString getSuffix() override{
        return "stl;obj;3d";
    }
public:
    threeDHandler(){

        rea2::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([](rea2::stream<QString>* aInput){
           /* auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto stm = readStorage(rt, pth, aInput->scope()->data<QJsonObject>("config"), "ByteArray");
            auto txt = stm->scope()->data<QByteArray>("data");*/

            aInput->outs<QString>(QFileInfo(aInput->data()).baseName() == "" ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "3d");
            auto idx = aInput->scope()->data<double>("index");

            aInput->outs<QString>(aInput->data(), "update3DAttr_reagrid" + QString::number(int(idx)) + "_ide_3d");
        }, getSuffix());

        rea2::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([](rea2::stream<QString>* aInput){
            aInput->outs(rea2::Json("title", "warning", "text", "not supported yet!"), "c++_popMessage");
        }, getSuffix());
    }
};


static rea2::regPip<QString> create_3d_handler([](rea2::stream<QString>* aInput){
    static threeDHandler three_hdl;
    aInput->outs(aInput->data(), "create_3d_handler");
}, QJsonObject(), "create_handler");
