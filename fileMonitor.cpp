#include "rea.h"
#include <QFileInfo>
#include <QDateTime>

struct monitorInfo{
    monitorInfo(int aIndex, const QString& aRoot, const QString& aPath, const QJsonObject& aConfig, long long aTime) : index(aIndex), root(aRoot), path(aPath), config(aConfig), time(aTime){
        saved = false;
    }
    int index;
    QString root;
    QString path;
    QJsonObject config;
    long long time;
    bool saved;
};

class fileMonitor{
public:
    fileMonitor(){

        //forbid polling thread block mainthread
        rea2::pipeline::instance()->add<QString, rea2::pipePartial>([](rea2::stream<QString>* aInput){
            aInput->out();
        }, rea2::Json("name", "lastModified_monitor",
                     "thread", 40,
                     "befored", "lastModified"));

        rea2::pipeline::instance()->add<QString, rea2::pipePartial>([](rea2::stream<QString>* aInput){
            aInput->out();
        }, rea2::Json("name", "aws0lastModified_monitor",
                     "thread", 40,
                     "befored", "aws0lastModified"));

        rea2::pipeline::instance()->add<QString>([this](rea2::stream<QString>* aInput){
            auto scp = aInput->scope();
            auto idx = size_t(scp->data<double>("index"));
            if (idx >= m_monitor_list.size())
                m_monitor_list.resize(idx + 1);
            if (aInput->data() == "")
                m_monitor_list[idx] = nullptr;
            else{
                m_monitor_list[idx] = std::make_shared<monitorInfo>(int(idx),
                                                                    scp->data<QString>("root"),
                                                                    aInput->data(),
                                                                    scp->data<QJsonObject>("config"),
                                                                    QDateTime::currentDateTime().toMSecsSinceEpoch());
            }
        }, rea2::Json("name", "recordFileModified", "thread", 40));

        rea2::pipeline::instance()->add<int, rea2::pipeAsync>([this](rea2::stream<int>* aInput){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            for (auto i : m_monitor_list)
                if (i){
                    auto tm = rea2::pipeline::input(i->path, "", std::make_shared<rea2::scopeCache>()->cache("config", i->config), true)->asyncCall<long long>(i->root + "lastModified_monitor")->data();
                    if (tm > i->time){
                        i->time = tm;
                        if (i->saved)
                            i->saved = false;
                        else{
                            aInput->outs<QString>(i->path, "openWorkFile", QFileInfo(i->path).suffix())
                                    ->scope(true)
                                    ->cache<double>("index", i->index)
                                    ->cache("root", i->root)
                                    ->cache("config", i->config)
                                    ->cache("time", tm);
                        }
                    }else
                        i->time = tm;
                }
            aInput->outs(aInput->data(), "monitorFileModified");
        }, rea2::Json("name", "monitorFileModified", "thread", 40))
                ->execute(rea2::in(0));

        rea2::pipeline::instance()->add<QString, rea2::pipePartial>([this](rea2::stream<QString>* aInput){
            auto idx = size_t(aInput->scope()->data<double>("index"));
            if (m_monitor_list[idx])
                m_monitor_list[idx]->saved = true;
            aInput->out();
        }, rea2::Json("name", "forbidReloadOnce", "after", "saveWorkFile"));
    }
private:
    std::vector<std::shared_ptr<monitorInfo>> m_monitor_list;
};

static fileMonitor mn;
