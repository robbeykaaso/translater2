#include "rea.h"
#include "handler.h"
#include <QJsonDocument>
#include <QFileInfo>
#include <QDateTime>

class mideaHandler : private handler{
private:
    QString getSuffix() override{
        return "avi;mp4;mkv;midea";
    }
public:
    mideaHandler(){

        rea2::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea2::stream<QString>* aInput){
            aInput->outs(aInput->data(), "testFFM");
            /* auto pth = aInput->data();
             auto rt = aInput->scope()->data<QString>("root");
             auto stm = readStorage(rt, pth, aInput->scope()->data<QJsonObject>("config"), "ByteArray");
             auto txt = stm->scope()->data<QByteArray>("data");*/

             aInput->outs<QString>(QFileInfo(aInput->data()).baseName() == "" ? "" : aInput->data(), "qml_modelOpened")
                     ->scope()
                     ->cache<QString>("type", "midea");
             auto idx = aInput->scope()->data<double>("index");

             aInput->outs<QString>(aInput->data(), "updateMideaAttr_reagrid" + QString::number(int(idx)) + "_ide_midea");
        }, getSuffix());

        rea2::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([this](rea2::stream<QString>* aInput){
            aInput->outs(rea2::Json("title", "warning", "text", "not supported yet!"), "c++_popMessage");
        }, getSuffix(), rea2::Json("thread", 30));
    }
};

static rea2::regPip<QString> create_midea_handler([](rea2::stream<QString>* aInput){
    static mideaHandler midea_hdl;
    aInput->outs(aInput->data(), "create_midea_handler");
}, QJsonObject(), "create_handler");
