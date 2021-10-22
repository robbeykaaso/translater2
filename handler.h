#include "rea.h"
#include <QString>

class progressRead{
public:
    void start(int aSum);
    std::shared_ptr<rea2::stream<bool>> check(std::shared_ptr<rea2::stream<bool>> aStream);
    void report();
private:
    int m_sum;
    std::vector<QString> m_failed_list;
};

class handler{
public:
    virtual ~handler() = default;
    std::shared_ptr<rea2::stream<bool>> readStorage(const QString& aRoot, const QString& aPath, const QJsonObject& aConfig, const QString& aType);
#define dpstWrite(aType)    \
    std::shared_ptr<rea2::stream<bool>> write##aType(const QString& aRoot, const QString& aPath, const QJsonObject& aConfig, const Q##aType& aJson){ \
        return rea2::in(false, "", std::make_shared<rea2::scopeCache>(rea2::Json("path", aPath, \
                                                                              "config", aConfig))\
        ->cache<Q##aType>("data", aJson), true)->asyncCall(aRoot + STR(write##aType)); \
    }
    dpstWrite(JsonObject)
    dpstWrite(ByteArray)
    QString calcID(const QString& aPath, const QString& aRoot, const QJsonObject& aConfig);
protected:
    virtual QString getSuffix() = 0;
    bool checkValidSave(const QString& aSuffix);
private:

};
