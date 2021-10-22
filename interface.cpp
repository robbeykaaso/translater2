#include "rea.h"
#include "reaJS.h"
#include "storage.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QFileInfo>

#ifdef USEOPENCV
#include "plugin/opencv/util.h"
#endif

class customFsStorage : public rea2::fsStorage{
protected:
#ifdef USEOPENCV
    bool writeImage(const QString& aPath, const QImage& aData) override{
        auto pth = stgRoot(aPath);
        checkPath(pth);
        auto dt = QImage2cvMat(aData);
        return cv::imwrite(pth.toLocal8Bit().toStdString().data(), dt);
    }
    bool readImage(const QString& aPath, QImage& aData) override{
        auto dt = cv::imread((stgRoot(aPath)).toLocal8Bit().toStdString().data(),
                              cv::IMREAD_UNCHANGED);
        aData = cvMat2QImage(dt);
        return true;
    }
#endif
};

static rea2::regPip<QQmlApplicationEngine*> reg_js_linker([](rea2::stream<QQmlApplicationEngine*>* aInput){
    qmlRegisterType<rea2::environmentJS>("EnvJS", 1, 0, "EnvJS");
    //ref from: https://stackoverflow.com/questions/25403363/how-to-implement-a-singleton-provider-for-qmlregistersingletontype
    auto m = rea2::getDefaultPipelineName(), m_qml = rea2::getDefaultQMLPipelineName();
    rea2::connectPipelines({m, "js", m_qml, "qmljs", m, m_qml, m_qml, m});
    rea2::pipeline::instance("js");
    rea2::pipeline::instance("qmljs");

    static customFsStorage local_storage;
    local_storage.initialize();

    aInput->out();
}, rea2::Json("name", "install0_js"), "initRea");

//reagrid0_ide_image

#define extendTrigger2(aType, aPipe, aPipeline) \
    rea2::pipeline::instance()->add<aType>([](rea2::stream<aType>* aInput){ \
        aInput->out(); \
    }, rea2::Json("name", QString(aPipeline) + "_" + QString(aPipe), \
                 "befored", aPipe, \
                 "external", aPipeline))

#define extendListener2(aType, aPipe, aPipeline) \
    rea2::pipeline::instance()->find(aPipe) \
    ->nextF<aType>([](rea2::stream<aType>* aInput){ \
        aInput->out(); \
    }, "", rea2::Json("name", QString(aPipeline) + "_" + QString(aPipe), \
                     "external", aPipeline))

void extendQSGInterface(const QString& aName){
    //qsg
    extendTrigger2(QJsonArray, "updateQSGAttr_" + aName, "qml");
    extendListener2(QJsonArray, "QSGAttrUpdated_" + aName, "qml");
    extendTrigger2(QJsonArray, "updateQSGCtrl_" + aName, "qml");
    extendListener2(QJsonObject, "updateQSGMenu_" + aName, "qml");

    //edit
    extendTrigger2(QJsonArray, "deleteShapes_" + aName, "qml");
    extendTrigger2(QJsonObject, "moveShapes_" + aName, "qml");
    extendTrigger2(QJsonObject, "arrangeShapes_" + aName, "qml");
    extendTrigger2(QJsonObject, "copyShapes_" + aName, "qml");
    extendTrigger2(QJsonObject, "pasteShapes_" + aName, "qml");
    extendTrigger2(QJsonObject, "cancelShapes_" + aName, "qml");
    //draw
    extendTrigger2(double, "cancelDrawCircle_" + aName, "qml");
    extendTrigger2(double, "cancelDrawEllipse_" + aName, "qml");
    extendTrigger2(double, "cancelDrawLine_" + aName, "qml");
    extendTrigger2(double, "cancelDrawPoint_" + aName, "qml");
    extendTrigger2(double, "cancelDrawRect_" + aName, "qml");
    extendTrigger2(double, "cancelDrawPoly_" + aName, "qml");
    extendTrigger2(double, "cancelDrawPolyLast_" + aName, "qml");
    extendTrigger2(QJsonObject, "completeDrawPoly_" + aName, "qml");
    extendTrigger2(double, "cancelDrawFree_" + aName, "qml");
    extendTrigger2(double, "cancelDrawFreeLast_" + aName, "qml");
    extendTrigger2(QJsonObject, "completeDrawFree_" + aName, "qml");

    extendTrigger2(double, "cancelDrawLink_" + aName, "qml");
}

static rea2::regPip<QQmlApplicationEngine*> test_qsg([](rea2::stream<QQmlApplicationEngine*>* aInput){
    //interface adapter    
    static int sum = 0;
    rea2::pipeline::instance()->add<double>([](rea2::stream<double>* aInput){
        auto cnt = int(aInput->data());
        for (auto i = sum; i < cnt; ++i){
            rea2::pipeline::instance()->add<std::shared_ptr<rea2::pipeline*>>([i](rea2::stream<std::shared_ptr<rea2::pipeline*>>* aInput){
                auto nm = "js" + QString::number(i);
                *aInput->data() = new rea2::pipelineJS(nm);
                rea2::pipeline::instance()->updateOutsideRanges({nm});
            }, rea2::Json("name", "createjs" + QString::number(i) + "pipeline"));

            extendQSGInterface("reagrid" + QString::number(i) + "_ide_image");
            extendQSGInterface("reagrid" + QString::number(i) + "_ide_dpst_anno");
            extendQSGInterface("reagrid" + QString::number(i) + "_ide_optgph");
            rea2::pipeline::instance()->add<QJsonObject>([](rea2::stream<QJsonObject>* aInput){
                aInput->out();
            }, rea2::Json("name", "updateQSGSelects_reagrid" + QString::number(i) + "_ide_dpst_anno", "external", "qml"));

            auto pip = "js" + QString::number(i);
            /*rea2::pipeline::instance()->find("doCommand")
            ->nextF<bool>([](rea2::stream<bool>* aInput){
                aInput->out();
            }, "manual", rea2::Json("name", pip + "_doCommand",
                             "external", pip));*/

            rea2::pipeline::instance(pip);
        }
        sum = cnt;
        aInput->out();
    }, rea2::Json("name", "reagridCountChanged"));
    rea2::pipeline::instance()->add<bool>([](rea2::stream<bool>* aInput){
        aInput->outsB(aInput->data(), "doCommand", aInput->tag())
                ->outs(aInput->data(), "", aInput->tag());
    }, rea2::Json("name", "js_doCommand",
                 "external", "js"));
    extendTrigger(bool, doCommand, qml);
/*
    //storage
    extendTrigger(bool, readImage, js);
    rea2::pipeline::instance()->add<bool>([](rea2::stream<bool>* aInput){
        if (aInput->data()){
            auto img = aInput->scope()->data<QImage>("data");
            auto uri = "data:image/png;base64, " + rea2::QImage2Base64(img);
            aInput->scope()->cache<QString>("uri", uri);
        }
        aInput->out();
    }, rea2::Json("after", "js_readImage"));
    rea2::pipeline::instance()->add<bool>([](rea2::stream<bool>* aInput){
        auto uri = aInput->scope()->data<QString>("uri");
        aInput->scope()->cache<QImage>("data", Uri2QImage(uri));
        aInput->out();
    }, rea2::Json("name", "js_writeImage",
                 "aftered", "writeImage",
                 "external",  "js"));*/
    //storage
    //extendTrigger(bool, writeJsonObject, qml);

    rea2::pipeline::instance()->add<bool, rea2::pipePartial>([](rea2::stream<bool>* aInput){
        aInput->out();
    }, rea2::Json("name", "qml_writeJsonObject",
                 "befored", "writeJsonObject",
                 "external", "qml",
                 "thread", 10));
    rea2::pipeline::instance()->add<bool, rea2::pipeParallel>([](rea2::stream<bool>* aInput){
        aInput->out();
    }, rea2::Json("name", "qml_aws0writeJsonObject",
                 "befored", "aws0writeJsonObject",
                 "external", "qml"));

    rea2::pipeline::instance()->add<bool, rea2::pipePartial>([](rea2::stream<bool>* aInput){
        aInput->out();
    }, rea2::Json("name", "qml_readJsonObject",
                 "befored", "readJsonObject",
                 "external", "qml",
                 "thread", 10));
    rea2::pipeline::instance()->add<bool, rea2::pipeParallel>([](rea2::stream<bool>* aInput){
        aInput->out();
    }, rea2::Json("name", "qml_aws0readJsonObject",
                 "befored", "readJsonObject",
                 "external", "qml"));
    auto read_bytearray = [](rea2::stream<bool>* aInput){
        if (aInput->data()){
            auto dt = aInput->scope()->data<QByteArray>("data");
            aInput->scope()->cache<QString>("data", dt.toBase64());
        }
        aInput->out();
    };
    rea2::pipeline::instance()->add<bool, rea2::pipePartial>([](rea2::stream<bool>* aInput){
        aInput->out();
    }, rea2::Json("name", "js_readJsonObject",
                 "befored", "readJsonObject",
                 "external", "js",
                 "thread", 10));
    rea2::pipeline::instance()->add<bool, rea2::pipeParallel>([](rea2::stream<bool>* aInput){
        aInput->out();
    }, rea2::Json("name", "js_aws0readJsonObject",
                 "befored", "readJsonObject",
                 "external", "js"));
    rea2::pipeline::instance()->add<bool, rea2::pipePartial>(read_bytearray,
        rea2::Json("name", "js_readByteArray",
                  "befored", "readByteArray",
                  "external", "js",
                  "thread", 10
                  ));
    auto write_bytearray = [](rea2::stream<bool>* aInput){
        auto dt = aInput->scope()->data<QString>("data");
        aInput->scope()->cache<QByteArray>("data", QByteArray::fromBase64(dt.toLocal8Bit()));
        aInput->out();
    };
    rea2::pipeline::instance()->add<bool, rea2::pipePartial>(write_bytearray,
        rea2::Json("name", "js_writeByteArray",
                  "aftered", "writeByteArray",
                  "external", "js",
                  "thread", 11));
    auto only_out = [](rea2::stream<QString>* aInput){
        aInput->out();
    };
    rea2::pipeline::instance()->add<QString, rea2::pipePartial>(only_out,
        rea2::Json("name", "js_deletePath",
                  "befored", "deletePath",
                  "external", "js",
                  "thread", 11));
    auto list_files = [](rea2::stream<QString>* aInput){
        auto pths = aInput->scope()->data<std::vector<QString>>("data");
        QJsonArray ret;
        for (auto i : pths)
            ret.push_back(i);
        aInput->scope(true)->cache("data", ret);
        aInput->out();
    };
    rea2::pipeline::instance()->add<QString>(list_files,
        rea2::Json("name", "qml_listFiles",
                  "befored", "listFiles",
                  "external", "qml"));
    rea2::pipeline::instance()->add<QString>(list_files,
        rea2::Json("name", "qml_aws0listFiles",
                  "thread", 10,
                  "befored", "aws0listFiles",
                  "external", "qml"));

    rea2::pipeline::instance()->add<QString>(list_files,
        rea2::Json("name", "js_listFiles",
                  "befored", "listFiles",
                  "external", "js"));
    rea2::pipeline::instance()->add<bool, rea2::pipeParallel>(read_bytearray,
        rea2::Json("name", "js_aws0readByteArray",
                  "befored", "aws0readByteArray",
                  "external", "js"));
    rea2::pipeline::instance()->add<bool, rea2::pipeParallel>(write_bytearray,
        rea2::Json("name", "js_aws0writeByteArray",
                  "aftered", "aws0writeByteArray",
                  "external", "js"));
    rea2::pipeline::instance()->add<QString, rea2::pipeParallel>(only_out,
        rea2::Json("name", "js_aws0deletePath",
                  "befored", "aws0deletePath",
                  "external", "js",
                  "thread", 11));
    rea2::pipeline::instance()->add<QString>(list_files,
        rea2::Json("name", "js_aws0listFiles",
                  "thread", 10,
                  "befored", "aws0listFiles",
                  "external", "js"));
    //control
    rea2::pipeline::instance()->add<QString>([](rea2::stream<QString>* aInput){
        aInput->out();
    }, rea2::Json("name", "create_handler"));
    rea2::pipeline::instance()->add<QString>(only_out,
        rea2::Json("name", "qml_modelOpened",
                  "external", "qml"));
    rea2::pipeline::instance()->find("js_saveWorkFile")
            ->nextP(rea2::pipeline::instance()->add<QString, rea2::pipePartial>([](rea2::stream<QString>* aInput){
                        aInput->out();
                    }, rea2::Json("name", "saveWorkFile", "thread", 40)));
    rea2::pipeline::instance()->add<QString>(only_out,
        rea2::Json("name", "evalPyScript0"))
            ->next("evalPyScript");
}, QJsonObject(), "initRea");
