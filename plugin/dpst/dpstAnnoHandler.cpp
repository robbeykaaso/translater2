#include "rea.h"
#include "qsgBoard.h"
#include "dpstAnnoHandler.h"
#include <QImage>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>

static rea2::regPip<QString> create_dpst_anno_handler([](rea2::stream<QString>* aInput){
    static dpstAnnoHandler dpst_anno_hdl;
    aInput->outs(aInput->data(), "create_dpst_anno_handler");
}, QJsonObject(), "create_handler");

static rea2::regPip<QString> clearSelects([](rea2::stream<QString>* aInput){
    auto idx = aInput->scope()->data<double>("index");
    rea2::pipeline::instance()->run<double>("clearSelects_reagrid" + QString::number(int(idx)) + "_ide_dpst_anno");
    aInput->out();
}, rea2::Json("after", "openWorkFile"));

void dpstAnnoHandler::switchMode(const QString& aID, const QString& aMode){
    if (aMode == "roi")
        m_mode.insert(aID, std::make_shared<roiMode>(this));
    else if (aMode == "anno")
        m_mode.insert(aID, std::make_shared<annoMode>(this));
}

std::shared_ptr<rea2::stream<QString>> dpstAnnoHandler::getTaskInfo(const QString& aID, rea2::stream<QString>* aInput){
    auto info = rea2::in(aInput->data(), "", std::make_shared<rea2::scopeCache>(rea2::Json("root", aInput->scope()->data<QString>("root"),
                                                                            "config", aInput->scope()->data<QJsonObject>("config"))), true)->asyncCall("findTaskInfo");
    if (info->data() == aInput->data())
        return nullptr;
    return info;
}

std::shared_ptr<dpstMode> dpstAnnoHandler::getMode(const QString& aID, rea2::stream<QString>* aInput){
    if (!m_mode.contains(aID)){
        auto tsk = getTaskInfo(aID, aInput);
        switchMode(aID, tsk ? tsk->scope()->data<QString>("mode") : "anno");
    }
    return m_mode.value(aID);
}

dpstAnnoHandler::dpstAnnoHandler(){
    //m_mode = std::make_shared<annoMode>(this);

    rea2::pipeline::instance()->find("openWorkFile")
    ->nextF<QString>([this](rea2::stream<QString>* aInput){
        auto pth = aInput->data();
        auto rt0 = aInput->scope()->data<QString>("root");
        auto cfg0 = aInput->scope()->data<QJsonObject>("config");
        auto id = calcID(pth, rt0, cfg0);

        //switch mode or filter multi-opening
        auto md = aInput->scope()->data<QString>("mode");
        if (md != ""){
            if (getMode(id, aInput)->name() == md)
                return;
            switchMode(id, md);
        }else{
            auto tm = aInput->scope()->data<long long>("time");
            auto lst_tm = m_last_modified_time.contains(id) ? m_last_modified_time.value(id) : - 1;
            if (tm && tm <= lst_tm)
                return;
            m_last_modified_time.insert(id, tm);
        }

        //do open
        auto stm = readStorage(rt0, pth, cfg0, "JsonObject");
        auto mdl = stm->scope()->data<QJsonObject>("data");
        m_data.insert(id, mdl);
        auto close = QFileInfo(aInput->data()).baseName() == "";

        auto pths = mdl.value("path").toArray();
        auto lbls = mdl.value("caption").toArray();
        auto stg = mdl.value("storage").toObject();
        auto cfg = stg.value("config").toObject();
        auto rt = stg.value("root").toString();
        auto scp = aInput->scope();
        auto objs = getMode(id, aInput)->getAnnos(id, mdl, aInput);

        auto mp = rea2::in(QJsonArray(), "", nullptr, true)->asyncCall("c++_getViewMap")->data();
        auto invisible = rea2::in(QJsonObject(), "", nullptr, true)->asyncCall("c++_getLabelVisible")->data();
        QSet<int> occupied;
        progressRead prg;
        prg.start(pths.size());
        for (auto i = 0; i < pths.size(); ++i){
            auto idx = i < mp.size() ? mp[i].toInt() : 0;
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope(true)
                    ->cache<bool>("noCache", scp->data<bool>("noCache"))
                    ->cache<QString>("type", "dpst_anno")
                    ->cache<QJsonObject>("config", cfg0)
                    ->cache<QString>("root", rt0)
                    ->cache<double>("index", idx);
            occupied.insert(idx);

            QHash<QString, QImage> imgs;
            pth = pths[i].toString();

            auto stm2 = prg.check(readStorage(rt, pth, cfg, "Image"));
            if (stm2->data())
                imgs.insert(pth, stm2->scope()->data<QImage>("data"));

            auto w = imgs.size() ? imgs.begin()->width() : 100,
                    h = imgs.size() ? imgs.begin()->height() : 100;

            auto objects = rea2::Json(objs, "img_0", rea2::Json(
                                         "path", pth,
                                         "range", rea2::JArray(0, 0, w, h),
                                         "type", "image",
                                         "caption", lbls[i].toString()
                                         ));
            QJsonObject objs;
            for (auto j : objects.keys()){
                auto obj = objects.value(j).toObject();
                if (!invisible.value(obj.value("caption").toString()).toBool())
                    objs.insert(j, obj);
            }

            auto anno_mdl = rea2::Json("width", w,
                                      "height", h,
                                      "max_ratio", 100,
                                      "min_ratio", 0.01,
                                      "text", rea2::Json(
                                          "visible", true,
                                          "size", rea2::JArray(50, 30),
                                          "location", "top"
                                          ),
                                      "objects", objs);
            aInput->outs<QJsonArray>(QJsonArray(), "updateQSGAttr_reagrid" + QString::number(int(idx)) + "_ide_dpst_anno")
                    ->scope(true)
                    ->cache<QJsonObject>("model", rea2::Json(anno_mdl, "id", pth))
                    ->cache<QHash<QString, QImage>>("image", imgs);
        }
        prg.report();

        /*auto idx0 = int(aInput->scope()->data<double>("index"));
        if (!occupied.contains(idx0)){
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "text");
            aInput->outs<QString>(QJsonDocument(mdl).toJson(), "updateQuillAttr_reagrid" + QString::number(int(idx0)) + "_ide_text")
                    ->scope(true)
                    ->cache<QString>("suffix", "json");
        }*/
    }, getSuffix());

    rea2::pipeline::instance()->find("saveWorkFile")
    ->nextF<QString>([this](rea2::stream<QString>* aInput){
        auto pth = aInput->data();
        auto rt = aInput->scope()->data<QString>("root");
        auto cfg = aInput->scope()->data<QJsonObject>("config");
        auto id = calcID(pth, rt, cfg);
        if (aInput->scope()->data<QString>("viewtype") == "text"){
            auto dt = rea2::in<QString>("", "", nullptr, true)->asyncCall("getQuillText_" + aInput->scope()->data<QString>("name") + "_ide_text", false)->data();
            m_data.insert(id, QJsonDocument::fromJson(dt.toUtf8()).object());
            if (writeByteArray(rt, pth, cfg, dt.toUtf8())->data())
                aInput->outs(aInput->data(), "workFileSaved");
        }else if (aInput->scope()->data<QString>("viewtype") == "dpst_anno"){
            getMode(id, aInput)->saveWorkFile(id, aInput);
        }else{
            aInput->outs(rea2::Json("title", "warning", "text", "not supported yet!"), "c++_popMessage");
        }
    }, getSuffix(), rea2::Json("thread", 30));

    rea2::pipeline::instance()->find("clientOnline")->nextF<QString>([this](rea2::stream<QString>* aInput){
        auto scp = aInput->scope();
        m_job_path = QFileInfo(scp->data<QString>("path")).path() + "/";
        m_job_root = scp->data<QString>("root");
        m_job_config = scp->data<QJsonObject>("config");
        aInput->out();
    }, "", rea2::Json("name", "qml_clientOnline",
                     "external", "qml"));
}

QJsonObject annoMode::getAnnos(const QString& aID, const QJsonObject& aModel, rea2::stream<QString>* aInput){
    auto ret = aModel.value("objects").toObject();
    if (m_handler->m_job_path == "")
        return ret;
    auto tsk_info = m_handler->getTaskInfo(aID, aInput);
    if (tsk_info){
        auto sel_jobs = rea2::pipeline::input(QJsonArray(), "", nullptr, true)
                ->asyncCall("c++_joblist_reagrid" + QString::number(tsk_info->scope()->data<int>("index")) + "_ide_dpst_train_listViewSelected")->data();
        QFileInfo tsk_file = (tsk_info->data());
        if (sel_jobs.size()){
            auto pth = m_handler->m_job_path + sel_jobs[0].toString() + "/" + QFileInfo(aInput->data()).baseName() + ".json";
            auto ret_annos = m_handler->readStorage(m_handler->m_job_root, pth, m_handler->m_job_config, "JsonObject")->scope()->data<QJsonObject>("data").value("objects").toObject();
            for (auto i : ret_annos.keys())
                ret.insert(i, ret_annos.value(i));
        }
    }
    return ret;
}

void annoMode::saveWorkFile(const QString& aID, rea2::stream<QString>* aInput){
    auto pth = aInput->data();
    auto rt = aInput->scope()->data<QString>("root");
    auto cfg = aInput->scope()->data<QJsonObject>("config");
    auto dt = rea2::in<rea2::qsgModel*>(nullptr, "", nullptr, true)->asyncCall("getQSGModel_" + aInput->scope()->data<QString>("name") + "_ide_dpst_anno", false)->data();
    auto mdl = m_handler->m_data.value(aID);
    auto objs = dt->value("objects").toObject();
    auto img = objs.value("img_0").toObject();
    auto img_pth = img.value("path");
    auto lbls = mdl.value("caption").toArray();
    auto pths = mdl.value("path").toArray();
    for (auto i = 0; i < pths.size(); ++i)
        if (pths[i] == img_pth){
            lbls[i] = img.value("caption");
            break;
        }
    mdl.insert("caption", lbls);
    QJsonObject annos;
    for (auto i : objs.keys())
        if (!i.startsWith("img_") && !i.startsWith("result_"))
            annos.insert(i, objs.value(i));
    mdl.insert("objects", annos);
    m_handler->m_data.insert(aID, mdl);
    if (m_handler->writeJsonObject(rt, pth, cfg, mdl)->data())
        aInput->outs(aInput->data(), "workFileSaved");
}

QJsonObject roiMode::getAnnos(const QString& aID, const QJsonObject&, rea2::stream<QString>* aInput){
    auto tsk_info = m_handler->getTaskInfo(aID, aInput);
    if (!tsk_info)
        return QJsonObject();
    QString tsk_pth = tsk_info->data(), tsk_rt = tsk_info->scope()->data<QString>("root");
    QJsonObject tsk_cfg = tsk_info->scope()->data<QJsonObject>("config");

    auto tsk = m_handler->readStorage(tsk_rt, tsk_pth, tsk_cfg, "JsonObject")->scope()->data<QJsonObject>("data");
    if (!tsk.value("local_roi").toBool())
        return tsk.value("roi").toObject();
    else{
        QFileInfo info(tsk_pth);
        auto pth = info.path() + "/" + info.baseName() + "_task/" + QFileInfo(aInput->data()).baseName() + ".json";
        auto abs = m_handler->readStorage(tsk_rt, pth, tsk_cfg, "JsonObject")->scope()->data<QJsonObject>("data");
        return abs.value("roi").toObject();
    }
}

void roiMode::saveWorkFile(const QString& aID, rea2::stream<QString>* aInput){
    auto tsk_info = m_handler->getTaskInfo(aID, aInput);
    if (!tsk_info)
        return;
    QString tsk_pth = tsk_info->data(), tsk_rt = tsk_info->scope()->data<QString>("root");
    QJsonObject tsk_cfg = tsk_info->scope()->data<QJsonObject>("config");

    auto dt = rea2::in<rea2::qsgModel*>(nullptr, "", nullptr, true)->asyncCall("getQSGModel_" + aInput->scope()->data<QString>("name") + "_ide_dpst_anno", false)->data();
    auto objs = dt->value("objects").toObject();
    objs.remove("img_0");

    auto tsk = m_handler->readStorage(tsk_rt, tsk_pth, tsk_cfg, "JsonObject")->scope()->data<QJsonObject>("data");
    if (!tsk.value("local_roi").toBool()){
        tsk.insert("roi", objs);
        if (!m_handler->writeJsonObject(tsk_rt, tsk_pth, tsk_cfg, tsk)->data())
            aInput->outs(rea2::Json("title", "warning", "text", "save roi failed!"), "c++_popMessage");
    }else{
        QFileInfo info(tsk_pth);
        auto pth = info.path() + "/" + info.baseName() + "_task/" + QFileInfo(aInput->data()).baseName() + ".json";
        auto abs = m_handler->readStorage(tsk_rt, pth, tsk_cfg, "JsonObject")->scope()->data<QJsonObject>("data");
        abs.insert("roi", objs);
        if (!m_handler->writeJsonObject(tsk_rt, pth, tsk_cfg, abs))
            aInput->outs(rea2::Json("title", "warning", "text", "save roi failed!"), "c++_popMessage");
    }
}

class qsgPluginCustomSelect : public rea2::qsgBoardPlugin{
private:
    rea2::pipe0* m_can_be_select;
    rea2::pipe0* m_update_qsgselects;
    rea2::pipe0* m_forbid_transform_image;
    std::shared_ptr<rea2::qsgBoardPlugin> m_origin_select;
public:
    qsgPluginCustomSelect(const std::shared_ptr<rea2::qsgBoardPlugin> aSelect, const QJsonObject& aConfig) : qsgBoardPlugin(aConfig){
        m_origin_select = aSelect;
    }
    ~qsgPluginCustomSelect() override{
        rea2::pipeline::instance()->find("qsgObjectCanBeSelected_" + getParentName())->removeAspect(rea2::pipe0::AspectType::AspectAfter, m_can_be_select->actName());
        rea2::pipeline::instance()->remove(m_can_be_select->actName());
        rea2::pipeline::instance()->find("updateSelectedMask_" + getParentName())->removeNext(m_update_qsgselects->actName(), true, false);
        rea2::pipeline::instance()->find("updateQSGAttr_" + getParentName())->removeAspect(rea2::pipe0::AspectType::AspectBefore, m_forbid_transform_image->actName());
        rea2::pipeline::instance()->remove(m_forbid_transform_image->actName());
    }
protected:
    QString getName(rea2::qsgBoard* aParent = nullptr) override{
        auto ret = qsgBoardPlugin::getName(aParent);
        m_origin_select->getName(aParent);

        m_can_be_select = rea2::pipeline::instance()->add<bool>([](rea2::stream<bool>* aInput){
            auto tgt = aInput->scope()->data<rea2::qsgObject*>("target");
            aInput->setData(tgt->value("tag") != "result");
            aInput->out();
        }, rea2::Json("after", "qsgObjectCanBeSelected_" + getParentName()));

        m_update_qsgselects = rea2::pipeline::instance()->find("updateSelectedMask_" + getParentName())->nextF<rea2::pointList>([this](rea2::stream<rea2::pointList>* aInput){
            auto sel_objs = aInput->scope()->data<QSet<QString>>("selects");
            QJsonObject sels;
            auto objs = getQSGModel()->getQSGObjects();
            for (auto i : sel_objs){
                auto shp = objs.value(i);
                sels.insert(i, *shp);
            }
            QPointF tl, br;
            if (sel_objs.size() > 0){
                auto pts = aInput->data();
                tl = getTransNode()->matrix().map(pts[0]);
                br = getTransNode()->matrix().map(pts[2]);
            }
            aInput->outs<QJsonObject>(sel_objs.size() > 0 ? rea2::Json("bound", rea2::JArray(tl.x(), tl.y(), br.x(), br.y()), "shapes", sels) : QJsonObject(),
                                      "updateQSGSelects_" + getParentName());
        });

        m_forbid_transform_image = rea2::pipeline::instance()->add<QJsonArray>([](rea2::stream<QJsonArray>* aInput){
            auto mdys = aInput->data();
            for (auto i : mdys){
                auto mdy = i.toObject();
                if (mdy.value("obj").toString().startsWith("img_") && mdy.value("key") == QJsonArray({"transform"}))
                    return;
            }
            aInput->out();
        }, rea2::Json("before", "updateQSGAttr_" + getParentName()));

        return ret;
    }

    void beforeDestroy() override{
        qsgBoardPlugin::beforeDestroy();
        m_origin_select->beforeDestroy();
        rea2::pipeline::instance()->run<QJsonObject>("updateQSGSelects_" + getParentName(), QJsonObject(), "selObject");
    }

    void keyPressEvent(QKeyEvent *event) override{
        m_origin_select->keyPressEvent(event);
    }
    void mousePressEvent(QMouseEvent *event) override{
        m_origin_select->mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override{
        m_origin_select->mouseMoveEvent(event);
    }

    void wheelEvent(QWheelEvent *event) override{
        m_origin_select->wheelEvent(event);
    }

    void hoverMoveEvent(QHoverEvent *event) override{
        m_origin_select->hoverMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        m_origin_select->mouseReleaseEvent(event);
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> plugin_select2([](rea2::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginCustomSelect>(aInput->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, rea2::Json("name", "create_qsgboardplugin_selectc", "befored", "create_qsgboardplugin_select"));

const auto after_createQSGBoardpluginDrawRect = rea2::Json("after", "create_qsgboardplugin_drawrect");
const auto after_createQSGBoardpluginDrawEllipse = rea2::Json("after", "create_qsgboardplugin_drawellipse");
const auto after_createQSGBoardpluginDrawCircle = rea2::Json("after", "create_qsgboardplugin_drawcircle");
const auto after_CreateQSGBoardpluginDrawPoly = rea2::Json("after", "create_qsgboardplugin_drawpoly");
const auto after_CreateQSGBoardpluginDrawFree = rea2::Json("after", "create_qsgboardplugin_drawfree");

template <typename T>
class drawTraits{
public:
    virtual ~drawTraits(){}
    typedef T type;
    virtual QString tag(){
        return "";
    }
    virtual void breakPoint(){

    }
};

template <typename DrawTraits>
class qsgPluginCustomDrawAnno : public rea2::qsgBoardPlugin{
private:
    rea2::pipe0* m_monitor;
protected:
    std::shared_ptr<rea2::qsgBoardPlugin> m_origin_draw;
public:
    qsgPluginCustomDrawAnno(std::shared_ptr<rea2::qsgBoardPlugin> aOriginDraw, const QJsonObject& aConfig) : qsgBoardPlugin(aConfig){
        m_origin_draw = aOriginDraw;
    }
    ~qsgPluginCustomDrawAnno() override{
        //rea2::pipeline::instance()->find("updateQSGAttr_" + getParentName())->removeAspect(rea2::pipe0::AspectAfter, m_monitor->actName());
        rea2::pipeline::instance()->find("QSGAttrUpdated_" + getParentName())->removeNext(m_monitor->actName(), true, false);
    }
protected:
    QString getName(rea2::qsgBoard* aParent = nullptr) override {
        qsgBoardPlugin::getName(aParent);
        auto ret = m_origin_draw->getName(aParent);
        m_monitor = rea2::pipeline::instance()->find("QSGAttrUpdated_" + getParentName())->template nextF<QJsonArray>([this](rea2::stream<QJsonArray>* aInput){
            QJsonObject shps;
            auto dt = aInput->data();
            for (auto i : dt){
                auto mdy = i.toObject();
                if (mdy.value("type") == "add" && mdy.value("cmd").toBool()){
                    shps.insert(mdy.value("tar").toString(), QJsonObject());
                }
            }
            if (shps.size()){
                QPointF tl, br;
                for (auto i = shps.begin(); i != shps.end(); ++i){
                    auto bnd = getQSGModel()->getQSGObjects().value(i.key())->getBoundBox();
                    if (i == shps.begin()){
                        tl = getTransNode()->matrix().map(bnd.topLeft());
                        br = getTransNode()->matrix().map(bnd.bottomRight());
                    }else{
                        auto tmp_tl = getTransNode()->matrix().map(bnd.topLeft());
                        auto tmp_br = getTransNode()->matrix().map(bnd.bottomRight());
                        tl = QPointF(std::min(tl.x(), tmp_tl.x()), std::min(tl.y(), tmp_tl.y()));
                        br = QPointF(std::max(br.x(), tmp_br.x()), std::max(br.y(), tmp_br.y()));
                    }
                }
                rea2::pipeline::instance()->run<QJsonObject>("updateQSGSelects_" + getParentName(),
                                                rea2::Json("bound", rea2::JArray(tl.x(), tl.y(), br.x(), br.y()),
                                                          "shapes", shps,
                                                          "show_menu", true),
                                                DrawTraits().tag());
            }
        });
        return ret;
    }
    void beforeDestroy() override{
        rea2::pipeline::instance()->run<QJsonObject>("updateQSGSelects_" + getParentName(), QJsonObject(), DrawTraits().tag());
        m_origin_draw->beforeDestroy();
    }
    void wheelEvent(QWheelEvent *event) override{
        m_origin_draw->wheelEvent(event);
    }
    void hoverMoveEvent(QHoverEvent *event) override {
        m_origin_draw->hoverMoveEvent(event);
    }
    void keyPressEvent(QKeyEvent *event) override{
        m_origin_draw->keyPressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override{
        m_origin_draw->mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) override{
        m_origin_draw->mouseReleaseEvent(event);
       // if (event->button() == Qt::LeftButton)
        //    updateSelectsGUI();
    }
    void mousePressEvent(QMouseEvent *event) override{
        m_origin_draw->mousePressEvent(event);
        rea2::pipeline::instance()->run<QJsonObject>("updateQSGSelects_" + getParentName(), QJsonObject(), DrawTraits().tag());
    }
};

class drawRect : public drawTraits<rea2::polyObject>{
public:
    QString tag() override{
        return "drawRect";
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> draw_rect_anno([](rea2::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginCustomDrawAnno<drawRect>>(aInput->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, after_createQSGBoardpluginDrawRect);

class drawEllipse : public drawTraits<rea2::ellipseObject>{
public:
    QString tag() override{
        return "drawEllipse";
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> draw_ellipse_anno([](rea2::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginCustomDrawAnno<drawEllipse>>(aInput->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, after_createQSGBoardpluginDrawEllipse);

class drawCircle : public drawTraits<rea2::ellipseObject>{
public:
    QString tag() override{
        return "drawCircle";
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> draw_circle_anno([](rea2::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginCustomDrawAnno<drawCircle>>(aInput->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, after_createQSGBoardpluginDrawCircle);

class drawPolies : public drawTraits<rea2::polyObject>{
public:
    virtual QString completed() = 0;
};

class drawPoly : public drawPolies{
public:
    QString tag() override{
        return "drawPoly";
    }
    QString completed() override{
        return "completeDrawPoly_";
    }
};

template <typename DrawPolies>
class qsgPluginCustomDrawPolyAnno : public qsgPluginCustomDrawAnno<DrawPolies>{
public:
    qsgPluginCustomDrawPolyAnno(std::shared_ptr<rea2::qsgBoardPlugin> aOriginDraw, const QJsonObject& aConfig) : qsgPluginCustomDrawAnno<DrawPolies> (aOriginDraw, aConfig){

    }
    ~qsgPluginCustomDrawPolyAnno() override{
      //  rea2::pipeline::instance()->find(DrawPolies().completed() + getParentName(), false)->removeNext(m_completed->actName());
      //  rea2::pipeline::instance()->remove(m_completed->actName());
    }
protected:
    QString getName(rea2::qsgBoard* aParent = nullptr) override {
        auto ret = qsgPluginCustomDrawAnno<DrawPolies>::getName(aParent);
       /* m_completed = rea2::pipeline::instance()->find(DrawPolies().completed() + getParentName())
                          ->template nextF<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
                              if (!aInput->data().empty()){
                                  updateSelectsGUI(aInput->data());
                              }
                          });*/
        return ret;
    }
    void mouseReleaseEvent(QMouseEvent *event) override{
        qsgPluginCustomDrawAnno<DrawPolies>::m_origin_draw->mouseReleaseEvent(event);
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> draw_poly_anno([](rea2::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginCustomDrawPolyAnno<drawPoly>>(aInput->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, after_CreateQSGBoardpluginDrawPoly);

class drawFree : public drawPolies{
public:
    QString tag() override{
        return "drawFree";
    }
    QString completed() override{
        return "completeDrawFree_";
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> draw_free_anno([](rea2::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginCustomDrawPolyAnno<drawFree>>(aInput->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, after_CreateQSGBoardpluginDrawFree);
