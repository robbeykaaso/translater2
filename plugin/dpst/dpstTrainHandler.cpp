#include "rea.h"
#include "handler.h"
#include "reaRemote.h"
#include <QFileInfo>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QQmlApplicationEngine>

class taskInfo{
public:
    taskInfo(const QString& aPath, const QString& aRoot, const QJsonObject& aConfig, const QJsonObject& aModel, int aIndex){
        path = aPath;
        root = aRoot;
        config = aConfig;
        model = aModel;
        mode = "anno";
        index = aIndex;
    }
    QString path;
    QString root;
    QJsonObject config;
    QJsonObject model;
    QString mode;
    int index;
};

class dpstTrainHandler : private handler{
private:
    QString getSuffix() override{
        return "dpst_train;tr_dpst_train";
    }
    QJsonObject prepareAnnoListGUI(const QJsonArray& aList){
        QJsonArray dt;
        for (auto i : aList)
            dt.push_back(rea::Json("entry", rea::JArray(i)));
        return rea::Json("title", rea::JArray("id"), "data", dt, "append", true);
    }
    QJsonObject prepareAnnoListGUI(const QJsonObject& aModel){
        QJsonArray dt;
        auto imgs = aModel.value("images").toObject();
        for (auto i : imgs){
            auto dts = i.toObject().value("data").toArray();
            for (auto j : dts)
                dt.push_back(rea::Json("entry", rea::JArray(j)));
        }
        auto ret = rea::Json("title", rea::JArray("id"), "data", dt);
        if (dt.size())
            ret.insert("selects", QJsonArray({0}));
        return ret;
    }
    QString calcID(const QString& aRoot, const QJsonObject& aConfig){
        return QCryptographicHash::hash((aRoot + "_" + QJsonDocument(aConfig).toJson()).toUtf8(), QCryptographicHash::Md5).toHex();
    }
    void forbidReload(int aIndex){
        rea::pipeline::input<QString>("", "", std::make_shared<rea::scopeCache>(rea::Json("index", aIndex * 1.0)), true)->asyncCall("forbidReloadOnce");
    }
    bool addAnnos(const QJsonArray& aAnnos, const QString& aRoot, const QString& aPath, const QJsonObject& aConfig){
        auto id0 = handler::calcID(aPath, aRoot, aConfig);
        auto mdl = QJsonObject(m_data.value(id0)->model);
        auto imgs = mdl.value("images").toObject();
        auto id = calcID(aRoot, aConfig);
        auto stg = imgs.value(id).toObject();
        stg.insert("root", aRoot);
        stg.insert("config", aConfig);
        auto imgs2 = stg.value("data").toArray();

        for (auto i : aAnnos)
            imgs2.push_back(i);
        stg.insert("data", imgs2);
        imgs.insert(id, stg);
        mdl.insert("images", imgs);
        forbidReload(m_data.value(id0)->index);
        if (!writeJsonObject(aRoot, aPath, aConfig, mdl))
            return false;
        m_data.value(id0)->model = mdl;
        return true;
    }

    void service(){
        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            QFileInfo inf(aInput->data());
            auto dir = inf.dir().path() + "/" + inf.baseName() + "_anno/";
            auto fls = rea::in(QJsonArray(), "", nullptr, true)->asyncCall("js_getSelectedFiles")->data();
            auto dt = rea::in(rea::Json("title", "anno config", "content", rea::Json("channel", rea::Json("value", 1))), "", nullptr, true)->asyncCall("c++_setParam")->data();
            if (dt.size()){
                QJsonArray annos;
                auto ch = dt.value("channel").toInt();
                auto rt = aInput->scope()->data<QString>("root");
                auto cfg = aInput->scope()->data<QJsonObject>("config");
                for (auto i = 0; i < fls.size(); i += ch){
                    QJsonArray pths, lbls;
                    for (auto j = 0; j < ch; ++j){
                        pths.push_back(fls[i + j]);
                        lbls.push_back("");
                    }
                    auto anno = rea::Json("path", pths,
                                          "storage", rea::Json(
                                              "root", rt,
                                              "config", cfg
                                              ),
                                          "caption", lbls);
                    auto nm = rea::generateUUID();
                    auto anno_pth = dir + nm + ".dpst_anno";
                    if (writeJsonObject(rt, anno_pth, cfg, anno)->data()){
                        aInput->outs(anno_pth, "workFileSaved");
                        annos.push_back(nm);
                    }
                }
                if (annos.size()){
                    if (addAnnos(annos, rt, aInput->data(), cfg))
                        aInput->outs(prepareAnnoListGUI(annos), "imagelist_" + aInput->scope()->data<QString>("name") + "_updateListView", "generateAnnos");
                }
            }
        }, rea::Json("name", "generateAnnos"));

        rea::pipeline::instance()->add<QString>([](rea::stream<QString>* aInput){

        }, rea::Json("name", "deleteAnnos"));

        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            auto annos = aInput->scope()->data<QJsonArray>("annos");
            if (!annos.size())
                return;
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto rt = aInput->scope()->data<QString>("root");
            auto pth = aInput->scope()->data<QString>("path");
            auto info = QFileInfo(pth);
            rea::pipeline::instance()->run("c++_updateProgress", rea::Json("title", "setting...", "sum", annos.size()));
            for (auto i : annos){
                auto i_pth = info.path() + "/" + info.baseName() + "_task/" + i.toString() + ".json";
                auto abs = readStorage(rt, i_pth, cfg, "JsonObject")->scope()->data<QJsonObject>("data");
                abs.insert("stage", aInput->data());
                if (writeJsonObject(rt, i_pth, cfg, abs)->data())
                    aInput->outs(i_pth, "workFileSaved");
                rea::pipeline::instance()->run("c++_updateProgress", QJsonObject());
            }

        }, rea::Json("name", "setAnnoStage"));

        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            auto scp = aInput->scope();
            auto id = handler::calcID(scp->data<QString>("path"), scp->data<QString>("root"), scp->data<QJsonObject>("config"));
            if (scp->data<QString>("mode") != "")
                m_data.value(id)->mode = scp->data<QString>("mode");
            aInput->out();
        }, rea::Json("name", "setDpstAnnoMode",
                     "external", "qml"));

        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            auto pth = QFileInfo(aInput->data()).baseName();
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto id = calcID(rt, cfg);
            for (auto i : m_data){
                auto dt = i->model.value("images").toObject().value(id).toObject().value("data").toArray();
                if (rea::indexOfArray(dt, pth) >= 0){
                    aInput->setData(i->path)->scope()
                            ->cache("root", i->root)
                            ->cache("config", i->config)
                            ->cache("mode", i->mode)
                            ->cache("index", i->index);
                    break;
                }
            }
            aInput->out();
        }, rea::Json("name", "findTaskInfo"));

        rea::pipeline::instance()->add<bool>([this](rea::stream<bool>* aInput){
            auto scp = aInput->scope();
            auto pth = scp->data<QString>("path");
            auto rt = scp->data<QString>("root");
            auto cfg = scp->data<QJsonObject>("config");
            auto id = handler::calcID(pth, rt, cfg);
            auto mdl = QJsonObject(m_data.value(id)->model);
            mdl.insert("local_roi", aInput->data());
            forbidReload(m_data.value(id)->index);
            if (writeJsonObject(rt, pth, cfg, mdl)->data()){
                m_data.value(id)->model = mdl;
                aInput->out();
            }else
                aInput->setData(!aInput->data())->out();
        }, rea::Json("name", "setLocalROI"));

        /*rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto scp = aInput->scope();
            auto id = handler::calcID(scp->data<QString>("path"), scp->data<QString>("root"), scp->data<QJsonObject>("config"));
            auto mdl = m_data.value(id);
            auto inf = QFileInfo(mdl->path);
            aInput->scope()->cache("remote", true);
            aInput->setData(rea::Json("job", snowflakeID(),
                                      "dir", inf.path() + "/" + inf.baseName()))->out();
        }, rea::Json("name", "startTrain"))
        ->next("training");
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (!dt.value("err").toInt()){
                auto scp = aInput->scope();
                auto pth = scp->data<QString>("path");
                auto rt = scp->data<QString>("root");
                auto cfg = scp->data<QJsonObject>("config");
                auto id = handler::calcID(pth, rt, cfg);
                auto mdl = QJsonObject(m_data.value(id)->model);
                auto jobs = mdl.value("jobs").toObject();
                jobs.insert(dt.value("job").toString(), rea::Json("dir", dt.value("dir")));
                mdl.insert("jobs", jobs);
                forbidReload(m_data.value(id)->index);
                if (writeJsonObject(rt, pth, cfg, mdl)->data()){
                    m_data.value(id)->model = mdl;
                    aInput->outs(prepareJobListGUI(mdl, true), "joblist_" + aInput->scope()->data<QString>("name") + "_updateListView");
                }
            }else
                aInput->outs(rea::Json("title", "warning", "text", dt.value("msg")), "popMessage");
        });*/

        /*rea::pipeline::instance()->find("testServer")->nextF<QJsonObject>([](rea::stream<QJsonObject>* aInput){
            std::cout << aInput->data().value("hello").toString().toStdString() << std::endl;
        });*/
    }
public:
    dpstTrainHandler(){
        rea::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto stm = readStorage(rt, pth, cfg, "JsonObject");
            auto mdl = stm->scope()->data<QJsonObject>("data");
            auto idx = aInput->scope()->data<double>("index");
            m_data.insert(handler::calcID(pth, rt, cfg), std::make_shared<taskInfo>(pth, rt, cfg, mdl, idx));

            auto close = QFileInfo(aInput->data()).baseName() == "";
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "dpst_train");

            rea::pipeline::instance()->find("c++_imagelist_reagrid" + QString::number(idx) + "_ide_dpst_train_listViewSelected")
            ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
                auto dt = aInput->data();
                if (!dt.size())
                    return;
                auto id = dt[0].toString();
                auto pth = aInput->scope()->data<QString>("path");
                auto rt = aInput->scope()->data<QString>("root");
                auto cfg = aInput->scope()->data<QJsonObject>("config");

                auto info = QFileInfo(pth);
                auto ret = readStorage(rt, info.path() + "/" + info.baseName() + "_task/" + id + ".json", cfg, "JsonObject");
                aInput->outs(ret->scope()->data<QJsonObject>("data").value("stage").toString("none"));
            }, "manual", rea::Json("name", "c++_imagelist_reagrid" + QString::number(idx) + "_ide_dpst_train_listViewSelected_AnnoAbstract",
                                   "external", "qml",
                                   "type", "Partial"));

            aInput->outs<QJsonObject>(mdl, "updateDpstAttr_reagrid" + QString::number(idx) + "_ide_dpst_train")->scope()->cache("path", pth);
            aInput->outs(prepareAnnoListGUI(mdl), "imagelist_reagrid" + QString::number(idx) + "_ide_dpst_train" + "_updateListView");
            aInput->outs(QJsonArray(), "imagelist_reagrid" + QString::number(idx) + "_ide_dpst_train_listViewSelected", "manual");
            //aInput->outs(prepareJobListGUI(mdl), "joblist_reagrid" + QString::number(idx) + "_ide_dpst_train" + "_updateListView");
            //aInput->outs(QJsonArray(), "joblist_reagrid" + QString::number(idx) + "_ide_dpst_train_listViewSelected", "manual");
        }, getSuffix());

        rea::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            if (!checkValidSave(QFileInfo(pth).suffix()))
                return;
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto dt = m_data.value(handler::calcID(pth, rt, cfg))->model;
            if (writeJsonObject(rt, pth, cfg, dt)->data())
                aInput->outs(aInput->data(), "workFileSaved");
        }, getSuffix());

        service();
    }
private:
    QHash<QString, std::shared_ptr<taskInfo>> m_data;
};

static rea::regPip<QString> create_dpst_train_handler([](rea::stream<QString>* aInput){
    static dpstTrainHandler dpst_train_hdl;
    aInput->outs(aInput->data(), "create_dpst_train_handler");
}, QJsonObject(), "create_handler");

class server{
public:
    server(){
        rea::pipeline::instance()->find("clientBoardcast")->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto val = aInput->data().value("value");
            if (val == "socket connected"){
                m_detail = aInput->data().value("detail").toString();
                aInput->outs(rea::GetMachineFingerPrint(), "clientOnline")->scope(true)->cache("remote", true);
                aInput->outs(aInput->data());
            }else if (val == "socket unconnected"){
                m_detail = "";
                aInput->out();
            }else if (val == "from gui"){
                aInput->setData(rea::Json("detail", m_detail))->out();
            }
        }, "", rea::Json("name", "serverStated",
                         "external", "qml"));
    }
private:
    QString m_detail;
};

static server sv;
