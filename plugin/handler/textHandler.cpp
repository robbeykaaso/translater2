#include "rea.h"
#include "handler.h"
#include <QJsonDocument>
#include <QFileInfo>
#include <QDateTime>

class textHandler : private handler{
private:
    QString getSuffix() override{
        return "txt;json;xml;text;tr_text";
    }
public:
    textHandler(){

        rea::pipeline::instance()->add<QString>([](rea::stream<QString>* aInput){
            std::cout << aInput->data().toStdString() << std::endl;
        }, rea::Json("name", "logJS"));

        rea::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto stm = readStorage(rt, pth, cfg, "ByteArray");
            auto txt = stm->scope()->data<QByteArray>("data");
            auto close = QFileInfo(aInput->data()).baseName() == "";
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "text");
            auto idx = aInput->scope()->data<double>("index");

            aInput->outs<QString>(txt, "updateQuillAttr_reagrid" + QString::number(int(idx)) + "_ide_text")
                    ->scope(true)
                    ->cache<QString>("suffix", QFileInfo(pth).suffix());
        }, getSuffix());

        rea::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            if (!checkValidSave(QFileInfo(pth).suffix()))
                return;
            auto rt = aInput->scope()->data<QString>("root");
            auto dt = rea::in<QString>("", "", nullptr, true)->asyncCall("getQuillText_" + aInput->scope()->data<QString>("name") + "_ide_text")->data();
            auto ok = rea::in(false, "", std::make_shared<rea::scopeCache>(rea::Json("path", pth,
                                                                           "config", aInput->scope()->data<QJsonObject>("config")))
                               ->cache<QByteArray>("data", dt.toUtf8()), true)
                    ->asyncCall(rt + "writeByteArray")->data();
            if (ok)
                aInput->outs(aInput->data(), "workFileSaved");
        }, getSuffix(), rea::Json("thread", 30));
    }
};

static rea::regPip<QString> create_text_handler([](rea::stream<QString>* aInput){
    static textHandler txt_hdl;
    aInput->outs(aInput->data(), "create_text_handler");
}, QJsonObject(), "create_handler");