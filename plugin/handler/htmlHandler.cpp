#include "rea.h"
#include "handler.h"
#include <QFileInfo>
#include <QDateTime>

class htmlHandler : private handler{
private:
    QString getSuffix() override{
        return "html;tr_html";
    }
public:
    htmlHandler(){
        rea2::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea2::stream<QString>* aInput){
            auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto stm = readStorage(rt, pth, cfg, "ByteArray");
            auto txt = stm->scope()->data<QByteArray>("data");
            auto close = QFileInfo(aInput->data()).baseName() == "";
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "html");
            auto idx = aInput->scope()->data<double>("index");

            aInput->outs<QString>(txt, "updateHtmlAttr_reagrid" + QString::number(int(idx)) + "_ide_html")
                    ->scope(true)
                    ->cache<QString>("type", "data");
        }, getSuffix());

        rea2::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([](rea2::stream<QString>* aInput){
            aInput->outs(rea2::Json("title", "warning", "text", "not supported yet!"), "c++_popMessage");
        }, getSuffix());
    }
};

static rea2::regPip<QString> create_html_handler([](rea2::stream<QString>* aInput){
    static htmlHandler html_hdl;
    aInput->outs(aInput->data(), "create_html_handler");
}, QJsonObject(), "create_handler");
