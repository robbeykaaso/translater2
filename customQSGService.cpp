#include "qsgBoard.h"
#include "optGraph.h"

class showPosPixelAspect : public rea2::qsgPluginTransform{
public:
    showPosPixelAspect(const QJsonObject& aConfig) : rea2::qsgPluginTransform(aConfig){}
    ~showPosPixelAspect() override{

    }
    QString getName(rea2::qsgBoard* aParent = nullptr) override{
        auto ret = qsgPluginTransform::getName(aParent);
        rea2::pipeline::instance()->add<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto x = dt.value("x").toDouble(),
                    y = dt.value("y").toDouble();
            auto ret = rea2::JArray("x: " + QString::number(int(x)) + " y: " + QString::number(int(y)));
            QString rgb = "";
            auto mdl = getQSGModel();
            if (mdl){
                auto objs = mdl->getQSGObjects();
                for (auto i : objs.keys()){
                    if (i.startsWith("img_")){
                        auto img_obj = reinterpret_cast<rea2::imageObject*>(objs.value(i).get());
                        auto img = img_obj->getImage();
                        auto pos0 = rea2::qsgObject::getTransform(*img_obj).inverted().map(QPointF(x, y));
                        auto pos = QPoint(int(pos0.x()), int(pos0.y()));
                        if (pos.x() < img.width() && pos.x() >= 0 && pos.y() < img.height() && pos.y() >= 0){
                            auto px = img.pixelColor(pos.x(), pos.y());
                            rgb += "r: " + QString::number(px.red()) +
                                   " g: " + QString::number(px.green()) +
                                   " b: " + QString::number(px.blue()) +
                                   " a: " + QString::number(px.alpha());
                            ret.push_back(rgb);
                        }
                    }
                }
            }

            aInput->outs(ret, "_updateStatus");
        }, rea2::Json("name", "updateQSGPos_" + getParentName(), "replace", true));
        return ret;
    }
};

static rea2::regPip<QJsonObject, rea2::pipePartial> create_qsgboardplugin_showPosPixelAspect([](rea2::stream<QJsonObject>* aInput){
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", std::make_shared<showPosPixelAspect>(aInput->data()));
    aInput->out();
}, rea2::Json("name", "create_qsgboardplugin_transform"));

class changeImageShowAspect : public rea2::qsgBoardPlugin{
private:
    rea2::pipe0* m_setImageShow;
    rea2::pipe0* m_setImageResizeFormat;
    rea2::pipe0* m_update_qsgselects;
    std::shared_ptr<rea2::qsgBoardPlugin> m_origin_select;
    QHash<QString, std::pair<rea2::imageObject*, QImage>> m_selects;
public:
    changeImageShowAspect(const std::shared_ptr<rea2::qsgBoardPlugin> aSelect, const QJsonObject& aConfig) : qsgBoardPlugin(aConfig){
        m_origin_select = aSelect;
    }
    ~changeImageShowAspect() override{
        rea2::pipeline::instance()->find("setImageShow")->removeNext(m_setImageShow->actName(), true, false);
        rea2::pipeline::instance()->find("setImageResizeFormat")->removeNext(m_setImageResizeFormat->actName(), true, false);
        rea2::pipeline::instance()->find("updateSelectedMask_" + getParentName())->removeNext(m_update_qsgselects->actName(), true, false);
    }
protected:
    QString getName(rea2::qsgBoard* aParent = nullptr) override{
        auto ret = qsgBoardPlugin::getName(aParent);
        m_origin_select->getName(aParent);
        m_setImageResizeFormat = rea2::pipeline::instance()->find("setImageResizeFormat")->nextF<QString>([this](rea2::stream<QString>* aInput){
            auto sw = aInput->scope()->data<bool>("show");
            for (auto i : m_selects.keys()){
                if (sw)
                    aInput->outs(rea2::JArray(rea2::Json("obj", i,
                                                       "key", rea2::JArray("resize_method"),
                                                       "val", aInput->data(),
                                                       "force", true)), "updateQSGAttr_" + getParentName());
                else
                    m_selects.value(i).first->insert("resize_method", aInput->data());
            }
        });
        m_setImageShow = rea2::pipeline::instance()->find("setImageShow")->nextF<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
            std::vector<QImage> imgs;
            QJsonArray idxes, imgids;
            int idx = 0;
            for (auto i : m_selects.keys()){
                imgids.push_back(i);
                imgs.push_back(m_selects.value(i).second);
                idxes.push_back(idx++);
            }

            std::vector<std::pair<QString, QJsonObject>> operators;
            std::vector<QJsonArray> nxts;
            int step = 0;
            auto dt = aInput->data();
            for (auto i : dt.keys()){
                auto id = "id" + QString::number(step++);
                operators.push_back(std::pair<QString, QJsonObject>(id, rea2::Json(dt.value(i).toObject(),
                                                                                  "type", i,
                                                                                  "imageIndex", idxes)));
                nxts.push_back(QJsonArray({id}));
            }
            auto id = "id" + QString::number(step++);
            operators.push_back(std::pair<QString, QJsonObject>(id, rea2::Json(
                                                                    "type", "updateImagePath",
                                                                    "value", imgids,
                                                                    "imageIndex", idxes,
                                                                    "view", getParentName())));
            nxts.push_back(QJsonArray({id}));
            QJsonObject mdy;
            for (size_t i = 0; i < operators.size(); ++i){
                if (!i)
                    operators[i].second.insert("input", 0);
                if (i < operators.size() - 1)
                    operators[i].second.insert("next", nxts[i + 1]);
                mdy.insert(operators[i].first, operators[i].second);
            }

            auto graph = std::make_shared<rea2::operatorGraph>();
            graph->build(mdy);
            graph->run(imgs);
        });
        m_update_qsgselects = rea2::pipeline::instance()->find("updateSelectedMask_" + getParentName())->nextF<rea2::pointList>([this](rea2::stream<rea2::pointList>* aInput){
            auto sels = aInput->scope()->data<QSet<QString>>("selects");
            m_selects.clear();
            for (auto i : sels)
                if (i.startsWith("img_")){
                    auto img = reinterpret_cast<rea2::imageObject*>(getQSGModel()->getQSGObjects().value(i).get());
                    m_selects.insert(i, std::pair<rea2::imageObject*, QImage>(img, img->getImage()));
                }
        });
        return ret;
    }

    void beforeDestroy() override{
        qsgBoardPlugin::beforeDestroy();
        m_origin_select->beforeDestroy();
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

static rea2::regPip<QString> setImageResizeFormat([](rea2::stream<QString>* aInput){
    aInput->out();
}, rea2::Json("name", "setImageResizeFormat"));

static rea2::regPip<QJsonObject> setImageShow([](rea2::stream<QJsonObject>* aInput){
    aInput->out();
}, rea2::Json("name", "setImageShow"));

static rea2::regPip<QJsonObject, rea2::pipePartial> plugin_setImageShow([](rea2::stream<QJsonObject>* aInput){
    auto stm = rea2::pipeline::instance()->call("create_qsgboardplugin_select", aInput->data(), aInput->scope(), false);
    auto plg = std::make_shared<changeImageShowAspect>(stm->scope()->data<std::shared_ptr<rea2::qsgBoardPlugin>>("result"), aInput->data());
    aInput->scope()->cache<std::shared_ptr<rea2::qsgBoardPlugin>>("result", plg);
    aInput->out();
}, rea2::Json("around", "create_qsgboardplugin_select"));
