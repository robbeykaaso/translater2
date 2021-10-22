#include "rea.h"
#include "util.h"
#include "reaRemote.h"
#include "storage.h"
#include <QQmlApplicationEngine>
#include <QTcpSocket>
#include <QDateTime>

class dl{
private:
    const QString stgPath = "F:/3M/default/jobs.json";
    const QJsonObject stgConfig = QJsonObject();
    const QString stgRoot = "";
    std::mutex m_job_mutex, m_socket_mutex;
private:
    bool startJob(const QString& aJob){
        if (m_running.size() > 4)
            return false;
        m_running.insert(aJob);
        auto cfg = m_jobs.value(aJob).toObject();
        rea2::pipeline::instance()->run<int>("doTraining", cfg.value("time").toInt(),
                                            "", std::make_shared<rea2::scopeCache>(rea2::Json(cfg,
                                                                                            "job", aJob,
                                                                                            "root0", stgRoot,
                                                                                            "path0", stgPath,
                                                                                            "config0", stgConfig,
                                                                                            "remote", true)));
        return true;
    }
private:
    void serviceTest(const QString& aExternal){
        /*rea2::pipeline::instance()->add<QJsonObject>([](rea2::stream<QJsonObject>* aInput){
            std::cout << "from client" << std::endl;
            aInput->setData(rea2::Json("hello", "world"))->out();
        }, rea2::Json("name", "testServer",
                     "external", aExternal));*/
    }
    const int whole_time = 1000;
    void serviceInternal(){
        /*rea2::pipeline::instance()->add<QString>([](rea2::stream<QString>* aInput){
            aInput->out();
        }, rea2::Json("name", "logTrain",
                     "external", "qml"));*/

        rea2::pipeline::instance()->add<int, rea2::pipeParallel>([this](rea2::stream<int>* aInput){
            auto tm = aInput->data();
            if (tm >= whole_time){
                auto job = aInput->scope()->data<QString>("job");
                std::lock_guard<std::mutex> lg(m_job_mutex);
                auto cfg = m_jobs.value(job).toObject();
                cfg.insert("status", "success");
                m_jobs.insert(job, cfg);
                while (!writeJsonObject(stgRoot, stgPath, stgConfig, m_jobs)->data())
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                m_running.remove(job);
            }else{
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                tm += 1;
                aInput->outs(tm, "doTraining");
            }
            auto usr = aInput->scope()->data<QString>("user");
            std::lock_guard<std::mutex> lg(m_socket_mutex);
            if (m_clients.contains(usr)){
                rea2::pipeline::instance()->run("logTrain", tm < whole_time ? "time cost: " + QString::number(tm) : "complete",
                                               "", aInput->scope()->cache("socket", m_clients.value(usr)));
            }
        }, rea2::Json("name", "doTraining"));
    }
public:
    dl(){
        serviceTest("c++");
        serviceTest("qml");
        serviceInternal();

        auto stm = readStorage(stgRoot, stgPath, stgConfig, "JsonObject");
        if (stm->data()){
            m_jobs = stm->scope()->data<QJsonObject>("data");
            for (auto i : m_jobs.keys()){
                auto job = m_jobs.value(i).toObject();
                if (job.value("status") == "running")
                    if (!startJob(i))
                        break;
            }
        }

        rea2::pipeline::instance()->add<QString>([this](rea2::stream<QString>* aInput){
            std::lock_guard<std::mutex> lg(m_socket_mutex);
            m_clients.insert(aInput->data(), aInput->scope()->data<QTcpSocket*>("socket"));
            aInput->scope()->cache("config", stgConfig)
                    ->cache("root", stgRoot)
                    ->cache("path", stgPath);
            aInput->out();
        }, rea2::Json("name", "clientOnline",
                     "external", "c++"));

        rea2::pipeline::instance()->find("clientStatusChanged")->nextF<bool>([this](rea2::stream<bool>* aInput){
            if (!aInput->data()){
                std::lock_guard<std::mutex> lg(m_socket_mutex);
                for (auto i : m_clients.keys())
                    if (aInput->scope()->data<QTcpSocket*>("socket") == m_clients.value(i)){
                        m_clients.remove(i);
                        break;
                    }
            }
        });

        rea2::pipeline::instance()->add<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
            auto usr = getUser(aInput->scope()->data<QTcpSocket*>("socket"));
            if (usr == ""){
                aInput->setData(rea2::Json(aInput->data(),
                                          "err", 1,
                                          "msg", "no this user!"))->out();
                return;
            }
            auto dt = aInput->data();
            auto job = snowflakeID();
            auto pth = dt.value("path").toString();
            auto root = dt.value("root").toString();
            auto cfg = dt.value("config").toObject();
            std::lock_guard<std::mutex> lg(m_job_mutex);
            m_jobs.insert(job, rea2::Json("path", pth,
                                         "root", root,
                                         "config", cfg,
                                         "user", usr,
                                         "status", "running"));
            if (writeJsonObject(stgRoot, stgPath, stgConfig, m_jobs)->data() && startJob(job)){
                aInput->setData(rea2::Json(aInput->data(), "err", 0))->out();
            }else{
                m_jobs.remove(job);
                aInput->setData(rea2::Json(aInput->data(),
                                          "err", 1,
                                          "msg", "start job failed!"))->out();
            }
        }, rea2::Json("name", "startTrain", "external", "qml"));
    }
private:
    QString snowflakeID(){
        return QString::number(QDateTime().currentDateTime().toTime_t()) + "-" + rea2::generateUUID();
    }
    QString getUser(QTcpSocket* aSocket){
        for (auto i : m_clients.keys())
            if (m_clients.value(i) == aSocket)
                return i;
        return "";
    }
    QHash<QString, QTcpSocket*> m_clients;
    QSet<QString> m_running;
    QJsonObject m_jobs;
};

static rea2::regPip<QQmlApplicationEngine*> reg_tcp_linker([](rea2::stream<QQmlApplicationEngine*>* aInput){
    static rea2::fsStorage stg;
    stg.initialize();
    static dl dl_;
    aInput->out();
}, rea2::Json("name", "install10_server"), "initRea");
