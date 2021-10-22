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

        rea2::pipeline::instance()->add<QString>([](rea2::stream<QString>* aInput){
            std::cout << aInput->data().toStdString() << std::endl;
        }, rea2::Json("name", "logJS"));

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
                    ->cache<QString>("type", "text");
            auto idx = aInput->scope()->data<double>("index");

            aInput->outs<QString>(txt, "updateQuillAttr_reagrid" + QString::number(int(idx)) + "_ide_text")
                    ->scope(true)
                    ->cache<QString>("suffix", QFileInfo(pth).suffix());
        }, getSuffix());

        rea2::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([this](rea2::stream<QString>* aInput){
            auto pth = aInput->data();
            if (!checkValidSave(QFileInfo(pth).suffix()))
                return;
            auto rt = aInput->scope()->data<QString>("root");
            auto dt = rea2::in<QString>("", "", nullptr, true)->asyncCall("getQuillText_" + aInput->scope()->data<QString>("name") + "_ide_text")->data();
            auto ok = rea2::in(false, "", std::make_shared<rea2::scopeCache>(rea2::Json("path", pth,
                                                                           "config", aInput->scope()->data<QJsonObject>("config")))
                               ->cache<QByteArray>("data", dt.toUtf8()), true)
                    ->asyncCall(rt + "writeByteArray")->data();
            if (ok)
                aInput->outs(aInput->data(), "workFileSaved");
        }, getSuffix(), rea2::Json("thread", 30));
    }
};

static rea2::regPip<QString> create_text_handler([](rea2::stream<QString>* aInput){
    static textHandler txt_hdl;
    aInput->outs(aInput->data(), "create_text_handler");
}, QJsonObject(), "create_handler");
