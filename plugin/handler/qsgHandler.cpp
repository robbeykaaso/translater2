#include "rea.h"
#include "handler.h"
#include "qsgModel.h"
#include <QImage>
#include <QFileInfo>
#include <QDateTime>

class qsgHandler : private handler{
private:
    QString getSuffix() override{
        return "qsg;tr_qsg";
    }
public:
    qsgHandler(){
        rea2::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea2::stream<QString>* aInput){
            auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto stm = readStorage(rt, pth, cfg, "JsonObject");
            auto mdl = stm->scope()->data<QJsonObject>("data");
           // m_data.insert(pth, mdl);

            auto close = QFileInfo(aInput->data()).baseName() == "";
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "image");
            auto idx = aInput->scope()->data<double>("index");

            QHash<QString, QImage> imgs;
            auto objs = mdl.value("objects").toObject();
            progressRead prg;
            prg.start(objs.size());
            for (auto i : objs){
                auto obj = i.toObject();
                if (obj.value("type") == "image"){
                    auto pth = obj.value("path").toString();
                    auto stm2 = prg.check(readStorage(rt, pth, cfg, "Image"));
                    if (stm2->data())
                        imgs.insert(pth, stm2->scope()->data<QImage>("data"));
                }else
                    prg.check(nullptr);
            }
            prg.report();
            if (mdl.empty()){
                mdl = rea2::Json("width", 100,
                                "height", 100,
                                "max_ratio", 100,
                                "min_ratio", 0.01);
            }
            aInput->scope(true)
                    ->cache<QJsonObject>("model", rea2::Json(mdl, "id", pth))
                    ->cache<QHash<QString, QImage>>("image", imgs);
            aInput->outs<QJsonArray>(QJsonArray(), "updateQSGAttr_reagrid" + QString::number(int(idx)) + "_ide_image");
        }, getSuffix());

        rea2::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([this](rea2::stream<QString>* aInput){
            auto pth = aInput->data();
            if (!checkValidSave(QFileInfo(pth).suffix()))
                return;
            auto rt = aInput->scope()->data<QString>("root");
            auto dt = rea2::in<rea2::qsgModel*>(nullptr, "", nullptr, true)->asyncCall("getQSGModel_" + aInput->scope()->data<QString>("name") + "_ide_image", false)->data();
            auto ok = rea2::in(false, "", std::make_shared<rea2::scopeCache>(rea2::Json("path", pth,
                                                                           "config", aInput->scope()->data<QJsonObject>("config")))
                               ->cache<QJsonObject>("data", *dt), true)
                    ->asyncCall(rt + "writeJsonObject")->data();
            if (ok)
                aInput->outs(aInput->data(), "workFileSaved");
        }, getSuffix());
    }
private:
   // QHash<QString, QJsonObject> m_data;
};


static rea2::regPip<QString> create_qsg_handler([](rea2::stream<QString>* aInput){
    static qsgHandler qsg_hdl;
    aInput->outs(aInput->data(), "create_qsg_handler");
}, QJsonObject(), "create_handler");
