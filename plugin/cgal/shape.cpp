#include "qsgModel.h"

#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/squared_distance_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Segment_2 Segment_2;

double pointProjectToLine2D(const Point_2& aPoint, const Point_2& aLineStart, const Point_2& aLineEnd){
    K::Vector_2 vec_s = aPoint - aLineStart, vec_e = aLineEnd - aLineStart;
    return CGAL::scalar_product(vec_s, vec_e) / vec_e.squared_length();
}

class transformedAspect{
protected:
    void transformUpdated(){

    }
    bool pointIsInRange(const QMatrix4x4& aTransform, const std::vector<rea::pointList>& aPoints, const QPointF& aPoint, const double aRadius = 5){
        auto pt = aTransform.map(aPoint);
        auto tar = Point_2(pt.x(), pt.y());
        auto r = aTransform.data()[0] * 5;
        r *= r;
        for (auto i : aPoints)
            for (int j = 0; j < i.size() - 1; ++j){
                auto st = Point_2(i[j].x(), i[j].y()), ed = Point_2(i[j + 1].x(), i[j + 1].y());
                auto prm = pointProjectToLine2D(tar, st, ed);
                if (prm >= 0 && prm <= 1){
                    auto dis = CGAL::squared_distance(tar, Segment_2(st, ed));
                    if (dis < r)
                        return true;
                }
            }
        return false;
    }
};

class linkObject : public rea::polyObject, transformedAspect{
public:
    linkObject(const QJsonObject& aConfig) : polyObject(aConfig){

    }
    bool bePointSelected(double aX, double aY) override{
        return pointIsInRange(reinterpret_cast<QSGTransformNode*>(m_outline->parent())->matrix(), m_points, QPointF(aX, aY));
    }
protected:
    void updateTransform() override{
        shapeObject::updateTransform();
        transformUpdated();
    }
};

static rea::regPip<QJsonObject, rea::pipePartial> init_createlink([](rea::stream<QJsonObject>* aInput){
    aInput->scope()->cache<std::shared_ptr<rea::qsgObject>>("result", std::make_shared<linkObject>(aInput->data()));
    aInput->out();
}, rea::Json("name", "create_qsgobject_link"));
