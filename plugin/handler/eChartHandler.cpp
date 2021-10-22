#include "rea.h"
#include "handler.h"
#include <QFileInfo>
#include <QDateTime>

class echartHandler : private handler{
private:
    QString getSuffix() override{
        return "echart";
    }
public:
    echartHandler(){
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
                    ->cache<QString>("type", "echart");
            auto idx = aInput->scope()->data<double>("index");

            aInput->outs<QJsonObject>(mdl, "updateEChartAttr_reagrid" + QString::number(int(idx)) + "_ide_echart");
        }, getSuffix());

        rea2::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([](rea2::stream<QString>* aInput){
            aInput->outs(rea2::Json("title", "warning", "text", "not supported yet!"), "c++_popMessage");
        }, getSuffix());
    }
private:
   // QHash<QString, QJsonObject> m_data;
};

static rea2::regPip<QString> create_echart_handler([](rea2::stream<QString>* aInput){
    static echartHandler echart_hdl;
    aInput->outs(aInput->data(), "create_echart_handler");
}, QJsonObject(), "create_handler");
