#include "handler.h"
#include "../../operatorGraph.h"
#include "qsgModel.h"
#include "qsgBoard.h"
#include "command.h"
#include <QImage>
#include <QFileInfo>
#include <QDateTime>

class optGphHandler : private handler{
private:
    QString getSuffix() override{
        return "optgph;tr_optgph";
    }
private:
    struct link{
        link(const QString& aStart, const QString& aEnd, const QString& aID = "") : start(aStart), end(aEnd){
            id = aID;
            if (id == "")
                id = rea::generateUUID();
        }
        QString start, end, id;
        bool deleted = false;
    };
    struct optModel{
        optModel(){

        }
        QString type;
        QPointF center;
        std::vector<std::shared_ptr<link>> links;
        bool deleted = false;
    };
    struct model{
        model(){

        }
        int index;
        QHash<QString, optModel> operators;
        QHash<QString, std::shared_ptr<link>> links;
    };
    QHash<QString, model> m_gui_models;
    void service(){
        rea::pipeline::instance()->add<double>([this](rea::stream<double>* aInput){
            auto scp = aInput->scope();
            auto cfg = scp->data<QJsonObject>("config");
            auto rt = scp->data<QString>("root");
            auto pth = scp->data<QString>("path");
            auto id = handler::calcID(pth, rt, cfg);
            auto opts = m_data.value(id).value("operators").toObject();
            auto graph = std::make_shared<operatorGraph>(id);
            graph->build(opts);
            graph->run();
        }, rea::Json("name", "runOperatorGraph"));

        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            auto opt = aInput->data();
            auto prgs = aInput->scope()->data<double>("progress");
            auto id = aInput->scope()->data<QString>("id");
            auto mdl = rea::tryFind(&m_gui_models, id);
            aInput->outs(rea::JArray(rea::Json("obj", opt,
                                               "key", rea::JArray("color"),
                                               "val", prgs < 100 ? "green" : "red",
                                               "id", id)),
                         "updateQSGAttr_reagrid" + QString::number(mdl->index) + "_ide_optgph");
        }, rea::Json("name", "operatorGraphRunning"));

        rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto id = dt.value("id").toString();
            auto opt = dt.value("opt").toString();
            auto ct = QPointF(dt.value("x").toDouble(), dt.value("y").toDouble());
            auto brd = dt.value("board").toString();
            auto mdl = rea::tryFind(&m_gui_models, id);
            auto opt_mdl = rea::tryFind(&mdl->operators, opt);
            opt_mdl->center = ct;
            for (auto i : opt_mdl->links)
                if (!i->deleted){
                    QPointF pt1, pt2;
                    if (i->start == opt){
                        pt1 = ct;
                        pt2 = rea::tryFind(&mdl->operators, i->end)->center;
                    }else{
                        pt1 = rea::tryFind(&mdl->operators, i->start)->center;
                        pt2 = ct;
                    }
                    aInput->outs(rea::JArray(rea::Json("obj", i->id,
                                                       "key", rea::JArray("points"),
                                                       "val", rea::JArray(QJsonArray(), rea::JArray(pt1.x(), pt1.y(), pt2.x(), pt2.y())),
                                                       "id", id)), "updateQSGAttr_" + brd);
                }

        }, rea::Json("name", "operatorPosChanged"));

        rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto opt = dt.value("opt").toString();
            auto id = dt.value("id").toString();
            auto mdl = rea::tryFind(&m_gui_models, id);
            if (mdl->links.contains(opt)){
                if (dt.value("cmd").toBool())
                    mdl->links.value(opt)->deleted = true;
            }else{
                auto opt_mdl = rea::tryFind(&mdl->operators, opt);
                opt_mdl->deleted = true;
                auto brd = dt.value("board").toString();
                for (auto i : opt_mdl->links)
                    aInput->outs(rea::JArray(rea::Json("tar", i->id,
                                                       "key", rea::JArray("objects"),
                                                       "type", "del",
                                                       "id", id)), "updateQSGAttr_" + brd);
            }
        }, rea::Json("name", "shapeDeleted"));

        rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto opt = dt.value("opt").toString();
            auto id = dt.value("id").toString();
            auto mdl = rea::tryFind(&m_gui_models, id);
            if (dt.value("tag") == "link"){
                if (mdl->links.contains(opt)){
                    if (dt.value("cmd").toBool())
                        mdl->links.value(opt)->deleted = false;
                }else{
                    if (dt.contains("start") && dt.contains("end")){
                        auto st = dt.value("start").toString(), ed = dt.value("end").toString();
                        auto lnk = std::make_shared<link>(st, ed, opt);
                        mdl->links.insert(lnk->id, lnk);
                        rea::tryFind(&mdl->operators, st)->links.push_back(lnk);
                        rea::tryFind(&mdl->operators, ed)->links.push_back(lnk);
                    }
                }
            }else{
                auto opt_mdl = rea::tryFind(&mdl->operators, opt);
                opt_mdl->center = QPointF(dt.value("x").toDouble(), dt.value("y").toDouble());
                opt_mdl->type = dt.value("caption").toString();
                opt_mdl->deleted = false;
                auto id = dt.value("id").toString();
                auto brd = dt.value("board").toString();
                for (auto i : opt_mdl->links){
                    auto st_opt = rea::tryFind(&mdl->operators, i->start),
                            ed_opt = rea::tryFind(&mdl->operators, i->end);
                    if (!i->deleted && !st_opt->deleted && !ed_opt->deleted){
                        auto st = st_opt->center, ed = ed_opt->center;
                        aInput->outs(rea::JArray(rea::Json("tar", i->id,
                                                           "key", rea::JArray("objects"),
                                                           "type", "add",
                                                           "val", rea::Json(
                                                               "type", "link",
                                                               "tag", "link",
                                                               "points", rea::JArray(QJsonArray(), rea::JArray(st.x(), st.y(), ed.x(), ed.y())),
                                                               "color", "gray",
                                                               "arrow", rea::Json("visible", true)
                                                               ),
                                                           "id", id)), "updateQSGAttr_" + brd);
                    }
                }
            }
        }, rea::Json("name", "shapeAdded"));

        rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto id = aInput->scope()->data<QString>("id");
            auto opt = aInput->scope()->data<QString>("opt");
            auto mdl = rea::tryFind(&m_gui_models, id);
            auto lnks = mdl->links;
            auto cfg = m_data.value(id).value("operators").toObject().value(opt).toObject();

            cfg.insert("type", rea::tryFind(&mdl->operators, opt)->type);
            QJsonArray bf;
            for (auto i : lnks)
                if (!i->deleted && i->end == opt && !rea::tryFind(&mdl->operators, i->start)->deleted)
                    bf.push_back(i->start);
            cfg.insert("before", bf);
            aInput->setData(cfg)->out();
        }, rea::Json("name", "getOperatorConfig"));

        rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto id = aInput->scope()->data<QString>("id");
            auto opt = aInput->scope()->data<QString>("opt");
            auto mdl = m_data.value(id);
            auto opts = mdl.value("operators").toObject();
            auto src = opts.value(opt).toObject();
            auto tgt = aInput->data();
            for (auto i : tgt.keys())
                src.insert(i, tgt.value(i));
            opts.insert(opt, src);
            mdl.insert("operators", opts);
            m_data.insert(id, mdl);
        }, rea::Json("name", "setOperatorConfig"));
    }
public:
    optGphHandler(){
        rea::pipeline::instance()->find("openWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto stm = readStorage(rt, pth, cfg, "JsonObject");
            auto mdl = stm->scope()->data<QJsonObject>("data");
            auto idx = aInput->scope()->data<double>("index");
            auto id = handler::calcID(pth, rt, cfg);
            m_data.insert(id, mdl);

            auto close = QFileInfo(aInput->data()).baseName() == "";
            aInput->outs<QString>(close ? "" : aInput->data(), "qml_modelOpened")
                    ->scope()
                    ->cache<QString>("type", "optgph");

            m_gui_models.remove(id);
            auto mdl_cache = rea::tryFind(&m_gui_models, id);
            mdl_cache->index = int(idx);
            QJsonObject objs;
            rea::pointList pts;
            auto opts = mdl.value("operators").toObject();
            for (auto i : opts.keys()){
                auto opt = opts.value(i).toObject();
                auto pos = opt.value("qsg").toObject().value("pos").toArray();
                if (pos.size() != 4)
                    continue;
                auto lt = QPointF(pos[0].toDouble(), pos[1].toDouble()),
                        rb = QPointF(pos[2].toDouble(), pos[3].toDouble());
                auto opt_mdl = rea::tryFind(&mdl_cache->operators, i);
                opt_mdl->type = opt.value("type").toString();
                opt_mdl->center = (lt + rb) * 0.5;
                pts.push_back(lt);
                pts.push_back(rb);
                objs.insert(i, rea::Json("type", "poly",
                                         "points", rea::JArray(QJsonArray(), rea::JArray(lt.x(), lt.y(), lt.x(), rb.y(), rb.x(), rb.y(), rb.x(), lt.y(), lt.x(), lt.y())),
                                         "caption", opt.value("type")
                                         ));
                auto nxts = opt.value("next").toArray();
                for (auto j : nxts){
                    auto nxt = j.toString();
                    auto lnk = std::make_shared<link>(i, nxt);
                    mdl_cache->links.insert(lnk->id, lnk);
                    rea::tryFind(&mdl_cache->operators, i)->links.push_back(lnk);
                    rea::tryFind(&mdl_cache->operators, nxt)->links.push_back(lnk);
                }
            }
            for (auto i : mdl_cache->links.values()){
                auto st = rea::tryFind(&mdl_cache->operators, i->start)->center,
                        ed = rea::tryFind(&mdl_cache->operators, i->end)->center;
                objs.insert(i->id, rea::Json("type", "link",
                                             "tag", "link",
                                             "points", rea::JArray(QJsonArray(), rea::JArray(st.x(), st.y(), ed.x(), ed.y())),
                                             "color", "gray",
                                             "arrow", rea::Json("visible", true)
                                             ));
            }
            QRectF bnd(0, 0, 100, 100);
            if (pts.size())
                bnd = rea::calcBoundBox(pts);

            auto anno_mdl = rea::Json("width", bnd.width() * 1.2,
                                      "height", bnd.height() * 1.2,
                                      "max_ratio", 100,
                                      "min_ratio", 0.01,
                                      "text", rea::Json(
                                          "visible", true,
                                          "fontsize", 16,
                                          "location", "middle",
                                          "background", false
                                          ),
                                      "objects", objs);
            aInput->outs<QJsonArray>(QJsonArray(), "updateQSGAttr_reagrid" + QString::number(int(idx)) + "_ide_optgph")
                    ->scope(true)
                    ->cache<QJsonObject>("model", rea::Json(anno_mdl, "id", id));
            aInput->outs<QJsonObject>(mdl, "updateOptGphAttr_reagrid" + QString::number(idx) + "_ide_optgph")->scope()->cache("path", pth);
        }, getSuffix());

        rea::pipeline::instance()->find("saveWorkFile")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            auto pth = aInput->data();
            if (!checkValidSave(QFileInfo(pth).suffix()))
                return;
            auto rt = aInput->scope()->data<QString>("root");
            auto cfg = aInput->scope()->data<QJsonObject>("config");
            auto id = handler::calcID(pth, rt, cfg);
            auto dt = m_data.value(id);
            auto opts = dt.value("operators").toObject();
            QJsonObject ret_opts;
            auto mdl = rea::tryFind(&m_gui_models, id);
            for (auto i : mdl->operators.keys()){
                auto opt_mdl = mdl->operators.value(i);
                if (!opt_mdl.deleted){
                    auto ct = opt_mdl.center;
                    QJsonArray nxts;
                    for (auto j : opt_mdl.links)
                        if (!j->deleted && j->start == i && !rea::tryFind(&mdl->operators, j->end)->deleted)
                            nxts.push_back(j->end);
                    auto opt = opts.value(i).toObject();
                    auto seq = opt.value("seq").toArray();
                    for (auto j = seq.size() - 1; j >= 0; --j){
                        auto prv = seq[j].toString();
                        auto prv_opt = rea::tryFind(&mdl->operators, prv);
                        if (prv_opt->deleted){
                            seq.removeAt(j);
                            continue;
                        }
                        bool vld = false;
                        for (auto k : prv_opt->links)
                            if (!k->deleted && k->end == i){
                                vld = true;
                                break;
                            }
                        if (!vld)
                            seq.removeAt(j);
                    }
                    if (seq.size())
                        opt.insert("seq", seq);
                    ret_opts.insert(i, rea::Json(opt,
                                                 "type", opt_mdl.type,
                                                 "next", nxts,
                                                 "qsg", rea::Json("pos", rea::JArray(ct.x() - 50, ct.y() - 15, ct.x() + 50, ct.y() + 15))));
                }
            }
            dt.insert("operators", ret_opts);
            if (writeJsonObject(rt, pth, cfg, dt)->data()){
                m_data.insert(id, dt);
                aInput->outs(aInput->data(), "workFileSaved");
            }
        }, getSuffix());

        service();
    }
private:
    QHash<QString, QJsonObject> m_data;
};

static rea::regPip<QString> create_optgph_handler([](rea::stream<QString>* aInput){
    static optGphHandler optgph_hdl;
    aInput->outs(aInput->data(), "create_optgph_handler");
}, QJsonObject(), "create_handler");

class qsgPluginOptGphSelect : public rea::qsgBoardPlugin{
private:
    class createLink{
    private:
        QJsonArray getGeometry(){
            return rea::JArray(QJsonArray(),
                               QJsonArray({m_st.x(), m_st.y(),
                                           m_ed.x(), m_ed.y()}));
        }
        std::function<bool(void)> removeShape(const QString& aShape, bool aCommand = true){
            auto nm = m_parent->getParentName();
            auto mdl = m_parent->getQSGModel();
            if (mdl){
                auto id = m_parent->getQSGModel()->value("id");
                return [nm, aShape, aCommand, id](){
                    auto stm = rea::pipeline::instance()->run<QJsonArray>("updateQSGAttr_" + nm,
                                       {rea::Json("key", rea::JArray("objects"),
                                                 "type", "del",
                                                 "tar", aShape,
                                                 "cmd", aCommand,
                                                 "id", id)}, "delObject");
                    return !stm->scope()->data<bool>("fail");
                };
            }else
                return nullptr;
        }
        std::function<bool(void)> addPoly(const QString& aShape, const QJsonArray& aPoints, bool aCommand = true){
            auto nm = m_parent->getParentName();
            auto id = m_parent->getQSGModel()->value("id");
            return [nm, aShape, aPoints, aCommand, id](){
                auto stm = rea::pipeline::instance()->instance()->run<QJsonArray>("updateQSGAttr_" + nm,
                                   {rea::Json("key", rea::JArray("objects"),
                                              "type", "add",
                                              "tar", aShape,
                                              "val", rea::Json(
                                                        "type", "link",
                                                        "tag", "link",
                                                        "color", "gray",
                                                        "arrow", rea::Json("visible", true),
                                                        "points", aPoints),
                                              "cmd", aCommand,
                                              "id", id)}, "addLink");
                return !stm->scope()->data<bool>("fail");
            };
        }
        bool storePoly(const QString& aShape, const QJsonArray& aPoints){
            auto mdl = m_parent->getQSGModel();
            if (!mdl)
                return false;
            auto stm = rea::pipeline::instance()->run<QJsonArray>("updateQSGAttr_" + m_parent->getParentName(),
                                           rea::JArray(rea::Json("key", rea::JArray("objects"),
                                                                 "type", "del",
                                                                 "tar", aShape,
                                                                 "id", mdl->value("id")),
                                                       rea::Json("key", rea::JArray("objects"),
                                                                 "type", "add",
                                                                 "tar", aShape,
                                                                 "val", rea::Json("type", "link",
                                                                                  "tag", "link",
                                                                                  "color", "gray",
                                                                                  "arrow", rea::Json("visible", true),
                                                                                  "points", aPoints),
                                                                 "start", m_st_shp,
                                                                 "end", m_ed_shp,
                                                                 "cmd", true,
                                                                 "id", mdl->value("id")
                                                                            )));
            return !stm->scope()->data<bool>("fail");
        }
    public:
        createLink(qsgPluginOptGphSelect* aParent, const QPointF& aStart, const QString& aStartShape){
            m_st_shp = aStartShape;
            m_parent = aParent;
            m_st = aStart;
            m_ed = m_st;
            m_shape = rea::generateUUID();
            addPoly(m_shape, getGeometry(), false)();
        }
        ~createLink(){
            if (m_shape != ""){
                auto rm = removeShape(m_shape, false);
                if (rm)
                    rm();
            }
        }
        bool generateLink(const QPointF& aPoint, const QString& aEndShape){
            m_ed_shp = aEndShape;
            if (m_st.x() != aPoint.x() || m_st.y() != aPoint.y()){
                m_ed = aPoint;
                if (storePoly(m_shape, getGeometry())){
                    rea::pipeline::instance()->run<rea::ICommand>("addCommand",
                                                      rea::ICommand(addPoly(m_shape, getGeometry()),
                                                                    removeShape(m_shape)),
                                                      "manual");
                    m_shape = "";
                    return true;
                }
            }
            return false;
        }
        void mouseMoveEvent(const QPointF& aPos){
            m_ed = aPos;
            rea::pipeline::instance()->run<QJsonArray>("updateQSGAttr_" + m_parent->getParentName(),
                               {rea::Json("obj", m_shape,
                                         "key", QJsonArray({"points"}),
                                         "id", m_parent->getQSGModel()->value("id"),
                                         "val", getGeometry())},
                               "drawLine");
        }
    protected:
        QString m_shape, m_st_shp, m_ed_shp;
        QPointF m_st, m_ed;
        qsgPluginOptGphSelect* m_parent;
    };
    std::shared_ptr<createLink> m_create_link = nullptr;
    std::shared_ptr<rea::qsgObject> tryPickUpShape(QMouseEvent *event, QString& aShape){
        auto pos = getTransNode()->matrix().inverted().map(event->pos());
        auto objs = getQSGModel()->getQSGObjects();
        for (auto i : objs.keys()){
            auto obj = objs.value(i);
            if (obj->value("tag") != "link"){
                if (obj->getBoundBox().contains(pos)){
                    aShape = i;
                    return obj;
                }
            }
        }
        return nullptr;
    }
    bool tryStartCreateLink(QMouseEvent *event){
        if (event->buttons().testFlag(Qt::RightButton)){
            QString nm;
            auto shp = tryPickUpShape(event, nm);
            if (shp){
                m_create_link = std::make_shared<createLink>(this, shp->getBoundBox().center(), nm);
                return true;
            }
        }
        return false;
    }
    bool tryEndCreateLink(QMouseEvent *event){
        QString nm;
        auto shp = tryPickUpShape(event, nm);
        if (shp)
            return m_create_link->generateLink(shp->getBoundBox().center(), nm);
        return false;
    }
private:
    rea::pipe0* m_forbid_move_link;
    rea::pipe0* m_forbid_copy_link;
    rea::pipe0* m_updateSelectedMask;
    rea::pipe0* m_moveLinks;
    std::shared_ptr<rea::qsgBoardPlugin> m_origin_select;
    QSet<QString> m_selects;
    QRectF calcSelectsBound(const QSet<QString>& aSelects, const QMap<QString, std::shared_ptr<rea::qsgObject>>& aShapes){
        QRectF bnd;
        int idx = 0;
        for (auto i : aSelects){
            auto cur = reinterpret_cast<rea::shapeObject*>(aShapes.value(i).get())->getBoundBox();
            if (idx++){
                bnd = QRectF(QPointF(std::min(bnd.left(), cur.left()), std::min(bnd.top(), cur.top())),
                             QPointF(std::max(bnd.right(), cur.right()), std::max(bnd.bottom(), cur.bottom())));
            }else
                bnd = cur;
        }
        return bnd;
    }
public:
    qsgPluginOptGphSelect(const std::shared_ptr<rea::qsgBoardPlugin> aSelect, const QJsonObject& aConfig) : qsgBoardPlugin(aConfig){
        m_origin_select = aSelect;
    }
    ~qsgPluginOptGphSelect() override{
        rea::pipeline::instance()->find("updateQSGAttr_" + getParentName())->removeAspect(rea::pipe0::AspectType::AspectBefore, m_forbid_move_link->actName());
        rea::pipeline::instance()->remove(m_forbid_move_link->actName());
        rea::pipeline::instance()->find("updateQSGSelectMenu_" + getParentName())->removeAspect(rea::pipe0::AspectType::AspectAfter, m_forbid_copy_link->actName());
        rea::pipeline::instance()->remove(m_forbid_copy_link->actName());
        rea::pipeline::instance()->find("updateSelectedMask_" + getParentName())->removeAspect(rea::pipe0::AspectType::AspectAround, m_updateSelectedMask->actName());
        rea::pipeline::instance()->remove(m_updateSelectedMask->actName());
    }
protected:
    QString getName(rea::qsgBoard* aParent = nullptr) override{
        auto ret = qsgBoardPlugin::getName(aParent);
        m_origin_select->getName(aParent);

        m_forbid_copy_link = rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.size() && m_selects.size()){
                auto objs = getQSGModel()->getQSGObjects();
                for (auto i : m_selects)
                    if (objs.contains(i))
                        if (objs.value(i)->value("tag") == "link"){
                            auto mn = dt.value("menu").toArray();
                            for (auto j = 0; j < mn.size(); ++j){
                                if (mn[j].toObject().value("cap") == "copy"){
                                    mn.removeAt(j);
                                    dt.insert("menu", mn);
                                    aInput->setData(dt);
                                    return;
                                }
                            }
                        }
            }
        }, rea::Json("after", "updateQSGSelectMenu_" + getParentName()));

        m_forbid_move_link = rea::pipeline::instance()->add<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto mdys = aInput->data();
            auto mdl = getQSGModel();
            if (mdl)
                for (auto i : mdys){
                    auto mdy = i.toObject();
                    auto opt = mdy.value("obj").toString();
                    if (opt != "" && mdl->getQSGObjects().contains(opt) && mdl->getQSGObjects().value(opt)->value("tag") == "link" && mdy.value("key") == QJsonArray({"transform"})){
                        aInput->scope()->cache("fail", true);
                        return;
                    }
                }
            aInput->out();
        }, rea::Json("before", "updateQSGAttr_" + getParentName()));

        m_updateSelectedMask = rea::pipeline::instance()->add<QSet<QString>>([this](rea::stream<QSet<QString>>* aInput){
             rea::pointList pts;
             m_selects = aInput->data();
             auto mdl = getQSGModel();
             if (m_selects.size() > 0){
                 auto bnd = calcSelectsBound(m_selects, mdl->getQSGObjects());
                 pts.push_back(bnd.topLeft());
                 pts.push_back(bnd.topRight());
                 pts.push_back(bnd.bottomRight());
                 pts.push_back(bnd.bottomLeft());
                 pts.push_back(QPointF(bnd.right(), (bnd.top() + bnd.bottom()) * 0.5));
             }
             aInput->scope()->cache<QSet<QString>>("selects", m_selects);
             aInput->outs<rea::pointList>(pts);

             QJsonObject cfg;
             if (m_selects.size())
                cfg = rea::in(QJsonObject(), "", std::make_shared<rea::scopeCache>(rea::Json("id", mdl->value("id"), "opt", *m_selects.begin())))
                    ->asyncCall("getOperatorConfig")->data();
             aInput->outs(cfg, "updateQSGSelects_" + getParentName())->scope(true)->cache("id", mdl->value("id").toString())->cache("opt", m_selects.size() ? *m_selects.begin() : "");
        }, rea::Json("around", "updateSelectedMask_" + getParentName()));

        m_moveLinks = rea::pipeline::instance()->find("QSGAttrUpdated_" + getParentName())->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto mdys = aInput->data();
            for (auto i : mdys){
                auto mdy = i.toObject();
                if (mdy.value("key") == QJsonArray({"transform"})){
                    auto opt = mdy.value("obj").toString();
                    if (opt != ""){
                        auto ct = reinterpret_cast<rea::shapeObject*>(getQSGModel()->getQSGObjects().value(opt).get())->getBoundBox().center();
                        aInput->outs(rea::Json("id", mdy.value("id"),
                                               "opt", opt,
                                               "x", ct.x(),
                                               "y", ct.y(),
                                               "board", getParentName()), "operatorPosChanged");
                    }
                }else if (mdy.value("type") == "del"){
                    aInput->outs(rea::Json("id", mdy.value("id"),
                                           "opt", mdy.value("tar"),
                                           "cmd", mdy.value("cmd"),
                                           "board", getParentName()), "shapeDeleted");
                }else if (mdy.value("type") == "add"){
                    auto opt = mdy.value("tar").toString();
                    auto shp = getQSGModel()->getQSGObjects().value(opt);
                    if (shp){
                        auto ct = reinterpret_cast<rea::shapeObject*>(shp.get())->getBoundBox().center();
                        aInput->outs(rea::Json("id", mdy.value("id"),
                                               "opt", opt,
                                               "cmd", mdy.value("cmd"),
                                               "x", ct.x(),
                                               "y", ct.y(),
                                               "tag", shp->value("tag"),
                                               "caption", shp->value("caption"),
                                               "start", mdy.value("start"),
                                               "end", mdy.value("end"),
                                               "board", getParentName()), "shapeAdded");
                    }
                }
            }
        });

        return ret;
    }

    void beforeDestroy() override{
        qsgBoardPlugin::beforeDestroy();
        m_origin_select->beforeDestroy();
        rea::pipeline::instance()->run<QJsonObject>("updateQSGSelects_" + getParentName(), QJsonObject(), "selObject");
    }

    void keyPressEvent(QKeyEvent *event) override{
        m_origin_select->keyPressEvent(event);
    }
    void mousePressEvent(QMouseEvent *event) override{
        if (!tryStartCreateLink(event))
            m_origin_select->mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override{
        if (m_create_link)
            m_create_link->mouseMoveEvent(getTransNode()->matrix().inverted().map(event->pos()));
        else
            m_origin_select->mouseMoveEvent(event);
    }

    void wheelEvent(QWheelEvent *event) override{
        m_origin_select->wheelEvent(event);
    }

    void hoverMoveEvent(QHoverEvent *event) override{
        m_origin_select->hoverMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (m_create_link){
            if (!tryEndCreateLink(event))
                m_origin_select->mouseReleaseEvent(event);
            m_create_link = nullptr;
        }
        else
            m_origin_select->mouseReleaseEvent(event);
    }
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_select2([](rea::stream<QJsonObject>* aInput){
    auto plg = std::make_shared<qsgPluginOptGphSelect>(aInput->scope()->data<std::shared_ptr<rea::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, rea::Json("name", "create_qsgboardplugin_selectc2", "befored", "create_qsgboardplugin_select"));
