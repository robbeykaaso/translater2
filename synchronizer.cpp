#include "rea.h"
#include <QDebug>

class synchronizer{
public:
    synchronizer(){
        m_sync = {{
            -1, [](const QSet<QString>& aGroups, const QString& aIDE){
                for (auto i : aGroups)
                    rea::pipeline::instance()->find("QSGAttrUpdated_reagrid" + i + aIDE)
                          ->removeNext("sync_QSGAttrUpdated_reagrid" + i + aIDE);
            }},
            {1, [this](const QSet<QString>& aGroups, const QString& aIDE){
                 for (auto i : aGroups)
                    rea::pipeline::instance()->find("QSGAttrUpdated_reagrid" + i + aIDE)
                        ->nextF<QJsonArray>([i, this, aIDE](rea::stream<QJsonArray>* aInput){
                            auto mdys0 = aInput->data();
                            QJsonArray mdys;
                            for (auto i : mdys0){
                                auto mdy = i.toObject();
                                if (!mdy.contains("secondTrig")){
                                    mdy.remove("id");
                                    mdys.push_back(rea::Json(mdy, "secondTrig", true));
                                }
                            }
                            if (mdys.size()){
                                auto cur = i.toInt();
                                for (auto j = 0; j < m_sum; ++j)
                                    if (j != cur)
                                        aInput->outs(mdys, "updateQSGAttr_reagrid" + QString::number(int(j)) + aIDE);
                             }
                        }, "", rea::Json("name", "sync_QSGAttrUpdated_reagrid" + i + aIDE));
             }}
        };

        rea::pipeline::instance()->find("reagridCountChanged")->nextF<double>([this](rea::stream<double>* aInput){
            m_sum = std::max(int(aInput->data()), m_sum);
        });

        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            auto dt = aInput->data();
            dt = dt.mid(dt.indexOf("_ide_"), dt.length());
            auto last_sync = m_last_sync.value(dt);

            auto prm = rea::Json("title", "synchronize");
            QJsonObject cnt;
            for (auto i = 0; i < m_sum; ++i){
                auto idx = QString::number(i);
                cnt[idx] = rea::Json("value", last_sync.value(idx).isNull() ? 1 : last_sync.value(idx).toInt());
            }
            prm.insert("content", cnt);
            last_sync = rea::in(prm, "", nullptr, true)->asyncCall("c++_setParam")->data();
            m_last_sync.insert(dt, last_sync);

            QHash<int, QSet<QString>> grps;
            for (auto i : last_sync.keys())
                rea::tryFind(&grps, last_sync.value(i).toInt())->insert(i);
            for (auto i : grps.keys())
                if (m_sync.contains(i))
                    m_sync.value(i)(grps.value(i), dt);
                else
                    m_sync.value(- 1)(grps.value(i), dt);
        }, rea::Json("name", "_Synchronize"));
    }
private:
    QHash<QString, QJsonObject> m_last_sync;
    int m_sum = 1;
    QHash<int, std::function<void(const QSet<QString>&, const QString&)>> m_sync;
};

static synchronizer syn;
