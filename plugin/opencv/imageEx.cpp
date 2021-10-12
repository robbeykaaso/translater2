#include "plugin/opencv/util.h"
#include <QQuickItem>

class imageObjectEx : public rea::imageObject {
public:
    rea::IUpdateQSGAttr updateQSGAttr(const QString& aModification) override{
        if (aModification == "resize_method_"){
            return [this](QSGNode*){
                updateResizeShow();
            };
        }
        else
            return imageObject::updateQSGAttr(aModification);
    }
    void removeQSGNodes() override{
        if (m_node){
            m_node->parent()->removeChildNode(m_node);
            delete m_node;
            if (m_text){
                m_text->parent()->removeChildNode(m_text); //cannot delete here
                m_text = nullptr;
            }
            m_node = nullptr;
        }
    }
private:
    cv::Mat m_image;
    QMatrix4x4 m_ocs2wcs;
    void updateResizeShow(){
        if (!m_node->parent())
            return;

        QTransform org2ocs;
        auto rg = getRange(QImage());
        org2ocs.scale(rg.width() / m_image.cols, rg.height() / m_image.rows);
        auto wcs2scs = reinterpret_cast<QSGTransformNode*>(m_node->nextSibling())->matrix(),
                org2scs = wcs2scs * m_ocs2wcs * org2ocs,
                scs2ocs = org2scs.inverted();
        auto bnd_ocs = rea::calcBoundBox(rea::pointList({scs2ocs.map(QPointF(0, 0)),
                                                         scs2ocs.map(QPointF(m_window->width() - 1, 0)),
                                                         scs2ocs.map(QPointF(m_window->width() - 1, m_window->height() - 1)),
                                                         scs2ocs.map(QPointF(0, m_window->height() - 1))}));
        auto ocs_lt = QPointF(std::max(0.0, std::round(bnd_ocs.left())), std::max(0.0, std::round(bnd_ocs.top()))),
                ocs_rb = QPointF(std::min((m_image.cols - 1) * 1.0, std::round(bnd_ocs.right())), std::min((m_image.rows - 1) * 1.0, std::round(bnd_ocs.bottom())));
        do{
            if (ocs_rb.x() > ocs_lt.x() && ocs_rb.y() > ocs_lt.y()){
                auto cliped = m_image(cv::Rect(ocs_lt.x(), ocs_lt.y(), ocs_rb.x() - ocs_lt.x() + 1, ocs_rb.y() - ocs_lt.y() + 1));
                auto bnd_scs = rea::calcBoundBox(rea::pointList({org2scs.map(ocs_lt),
                                                                 org2scs.map(QPointF(ocs_lt.x(), ocs_rb.y())),
                                                                 org2scs.map(QPointF(ocs_rb.x(), ocs_lt.y())),
                                                                 org2scs.map(ocs_rb)}));
                auto scs_lt = QPointF(std::max(0.0, std::round(bnd_scs.left())), std::max(0.0, std::round(bnd_scs.top()))),
                        scs_rb = QPointF(std::min(m_window->width() - 1, std::round(bnd_scs.right())), std::min(m_window->height() - 1, std::round(bnd_scs.bottom())));
                org2scs.translate(ocs_lt.x(), ocs_lt.y());
                cv::Mat af(2, 3, CV_32FC1);
                auto src = org2scs.data();
                af.at<float>(0, 0) = src[0];
                af.at<float>(0, 1) = src[4];
                af.at<float>(0, 2) = src[12];
                af.at<float>(1, 0) = src[1];
                af.at<float>(1, 1) = src[5];
                af.at<float>(1, 2) = src[13];
                if (scs_rb.x() >= 0 && scs_rb.y() >= 0){
                    cv::Mat dst(scs_rb.y() + 1, scs_rb.x() + 1, m_image.type(), cv::Scalar(255, 255, 255, 255));
                    auto md = value("resize_method").toString("linear");
                    if (md == "neareast")
                        cv::warpAffine(cliped, dst, af, dst.size(), cv::InterpolationFlags::INTER_NEAREST, cv::BorderTypes::BORDER_TRANSPARENT);
                    else if (md == "cubic")
                        cv::warpAffine(cliped, dst, af, dst.size(), cv::InterpolationFlags::INTER_CUBIC, cv::BorderTypes::BORDER_TRANSPARENT);
                    else if (md == "area")
                        cv::warpAffine(cliped, dst, af, dst.size(), cv::InterpolationFlags::INTER_AREA, cv::BorderTypes::BORDER_TRANSPARENT);
                    else if (md == "lanczos4")
                        cv::warpAffine(cliped, dst, af, dst.size(), cv::InterpolationFlags::INTER_LANCZOS4, cv::BorderTypes::BORDER_TRANSPARENT);
                    else
                        cv::warpAffine(cliped, dst, af, dst.size(), cv::InterpolationFlags::INTER_LINEAR, cv::BorderTypes::BORDER_TRANSPARENT);
                    dst = dst(cv::Rect(scs_lt.x(), scs_lt.y(), scs_rb.x() - scs_lt.x() + 1, scs_rb.y() - scs_lt.y() + 1)).clone();
                    m_node->setTexture(m_window->window()->createTextureFromImage(cvMat2QImage(dst)));
                    auto ofst = QPointF(0.5, 0.5);
                    m_node->setRect(QRectF(scs_lt - ofst, scs_rb - ofst));
                    m_node->markDirty(QSGNode::DirtyMaterial);
                    return;
                }
            }
        }while(0);
        auto bk = QImage(m_window->width(), m_window->height(), QImage::Format_ARGB32);
        bk.fill(QColor("transparent"));
        m_node->setTexture(m_window->window()->createTextureFromImage(bk));
        m_node->setRect(bk.rect());
        m_node->markDirty(QSGNode::DirtyMaterial);
    }
public:
    imageObjectEx(const QJsonObject& aConfig) : imageObject(aConfig) {

    }
protected:
    QImage updateImagePath(bool aForce = false) override {
        QImage img = getImage();
        if (img.width() == 0 || img.height() == 0){
            if (!aForce)
                return img;
            img = QImage(10, 10, QImage::Format_ARGB32);
            img.fill(QColor("transparent"));
        }
        m_image = QImage2cvMat(img);
        updateResizeShow();
        return img;
    }
    void appendToParent(QSGNode* aTransformNode) override{
        aTransformNode->parent()->insertChildNodeBefore(m_node, aTransformNode);
    }
    void updateTransform() override {
        m_ocs2wcs = getTransform(*this);
        rea::pointList pts;
        auto rg = getRange(QImage());
        pts.push_back(rg.topLeft());
        pts.push_back(rg.topRight());
        pts.push_back(rg.bottomRight());
        pts.push_back(rg.bottomLeft());
        m_bound = calcTransformedBoundBox(pts, m_ocs2wcs);
        updateResizeShow();
    }
    void transformChanged() override {
        imageObject::transformChanged();
        updateResizeShow();
    }
};

static rea::regPip<QJsonObject> create_image([](rea::stream<QJsonObject>* aInput){
    aInput->scope()->cache<std::shared_ptr<rea::qsgObject>>("result", std::make_shared<imageObjectEx>(aInput->data()));
}, rea::Json("around", "create_qsgobject_image"));
