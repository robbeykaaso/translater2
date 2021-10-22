#ifndef SERVER_UTIL_H_
#define SERVER_UTIL_H_

#include "rea.h"

static std::shared_ptr<rea2::stream<bool>> readStorage(const QString& aRoot, const QString& aPath, const QJsonObject& aConfig, const QString& aType){
    auto stm = rea2::in(false, "", std::make_shared<rea2::scopeCache>(rea2::Json("path", aPath,
                                                                              "config", aConfig)), true)
            ->asyncCall(aRoot + "read" + aType);
    return stm;
}

#define dpstWrite(aType)    \
static std::shared_ptr<rea2::stream<bool>> write##aType(const QString& aRoot, const QString& aPath, const QJsonObject& aConfig, const Q##aType& aJson){ \
    return rea2::in(false, "", std::make_shared<rea2::scopeCache>(rea2::Json("path", aPath, \
                                                                          "config", aConfig))\
    ->cache<Q##aType>("data", aJson), true)->asyncCall(aRoot + STR(write##aType)); \
}
dpstWrite(JsonObject)

#endif
