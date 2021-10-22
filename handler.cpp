#include "handler.h"
#include <QCryptographicHash>
#include <QJsonDocument>


void progressRead::start(int aSum){
    if (aSum)
        rea2::pipeline::instance()->run("c++_updateProgress", rea2::Json("title", "loading...", "sum", aSum));
    m_sum = aSum;
    m_failed_list.clear();
}

std::shared_ptr<rea2::stream<bool>> progressRead::check(std::shared_ptr<rea2::stream<bool>> aStream){
    if (m_sum){
        rea2::pipeline::instance()->run("c++_updateProgress", QJsonObject());
        m_sum--;
    }
    if (aStream && !aStream->data())
        m_failed_list.push_back(aStream->scope()->data<QString>("path"));
    return aStream;
}

void progressRead::report(){
    if (m_failed_list.size()){
        QString ret = "failed list: \n";
        for (auto i : m_failed_list)
            ret += i + "\n";
        rea2::pipeline::instance()->run("popMessage", rea2::Json("title", "warning", "text", ret));
    }
}

bool handler::checkValidSave(const QString& aSuffix){
    if (getSuffix().indexOf(aSuffix) < 0){
        auto ok = rea2::in(rea2::Json("title", "warning",
                                    "text", "make sure to overwrite as ." + aSuffix + " file?"),
                          "", nullptr, true)
                ->asyncCall<bool>("c++_popMessage")->data();
        if (!ok)
            return false;
    }
    return true;
}

std::shared_ptr<rea2::stream<bool>> handler::readStorage(const QString& aRoot, const QString& aPath, const QJsonObject& aConfig, const QString& aType){
    auto stm = rea2::in(false, "", std::make_shared<rea2::scopeCache>(rea2::Json("path", aPath,
                                                                              "config", aConfig)), true)
            ->asyncCall(aRoot + "read" + aType);
    return stm;
}

QString handler::calcID(const QString& aPath, const QString& aRoot, const QJsonObject& aConfig){
    return QCryptographicHash::hash((aPath + "_" + aRoot + "_" + QJsonDocument(aConfig).toJson()).toUtf8(), QCryptographicHash::Md5).toHex();
}
