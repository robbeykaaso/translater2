#include "rea.h"
#include "handler.h"
#include <QImage>
#include <QFileInfo>
#include <QDateTime>
#include <QQmlApplicationEngine>

class imageHandler : private handler{
private:
    QString getSuffix() override{
        return "png;bmp;jpg;jpeg;ppm;gif;image";
    }
public:
    imageHandler(){
        rea::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto stm = readStorage(rt, pth, aInput->scope()->data<QJsonObject>("config"), "Image");
            auto img = stm->scope()->data<QImage>("data");
            auto close = QFileInfo(aInput->data()).baseName() == "";
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "image");
            auto idx = aInput->scope()->data<double>("index");

            QHash<QString, QImage> imgs;
            imgs.insert(pth, img);
            auto cfg = rea::Json("id", pth,
                                 "width", img.width() ? img.width() : 600,
                                 "height", img.height() ? img.height() : 600,
                                 "max_ratio", 100,
                                 "min_ratio", 0.01,
                                 "objects", rea::Json(
                                                "img_0", rea::Json(
                                                             "type", "image",
                                                             "range", rea::JArray(0, 0, img.width(), img.height()),
                                                             "path", pth)));
            aInput->scope(true)
                    ->cache<QJsonObject>("model", cfg)
                    ->cache<QHash<QString, QImage>>("image", imgs);
            aInput->outs<QJsonArray>(QJsonArray(), "updateQSGAttr_reagrid" + QString::number(int(idx)) + "_ide_image");
        }, getSuffix());

        rea::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([](rea::stream<QString>* aInput){
            aInput->outs(rea::Json("title", "warning", "text", "not supported yet!"), "c++_popMessage");
        }, getSuffix());
    }
};

static rea::regPip<QString> create_text_handler([](rea::stream<QString>* aInput){
    static imageHandler img_hdl;
    aInput->outs(aInput->data(), "create_image_handler");
}, QJsonObject(), "create_handler");

static rea::regPip<QString> clearSelects([](rea::stream<QString>* aInput){
    auto idx = aInput->scope()->data<double>("index");
    rea::pipeline::instance()->run<double>("clearSelects_reagrid" + QString::number(int(idx)) + "_ide_image");
    aInput->out();
}, rea::Json("after", "openWorkFile"));

static rea::regPip<QQmlApplicationEngine*> load_dialog([](rea::stream<QQmlApplicationEngine*>* aInput){
    aInput->data()->load("file:gui/service/image/dialog/ImageShow.qml");
    aInput->out();
},  rea::Json("before", "loadMain"));
