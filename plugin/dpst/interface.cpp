#include "reaRemote.h"
#include "client.h"
#include "ssdp.h"
#include <QQmlApplicationEngine>
#include <QTimer>

class searchServerAspect{
public:
    searchServerAspect(rea::normalClient* aClient);
    ~searchServerAspect();
private:
    void tryConnectServer();
    rea::DiscoveryManager ssdp_;
    QTimer search_timer_;
};

searchServerAspect::searchServerAspect(rea::normalClient* aClient){
    QObject::connect(&ssdp_, SIGNAL(FoundServer(QString, QString, QString)), aClient, SLOT(ServerFound(QString, QString, QString)));
    QObject::connect(&search_timer_, &QTimer::timeout, [this](){
        ssdp_.StartDiscovery();
        rea::pipeline::instance()->run<QJsonObject>("clientBoardcast", rea::Json("value", "search server..."));
    });

    rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        if (aInput->data().empty())
            tryConnectServer();
    }, rea::Json("after", "tryLinkServer"));

    rea::pipeline::instance()->find("clientBoardcast")->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        if (aInput->data().value("value") == "socket connected")
            search_timer_.stop();
        else if (aInput->data().value("value") == "socket unconnected")
            aInput->outs(QJsonObject(), "tryLinkServer");
    });
}

void searchServerAspect::tryConnectServer()
{
    if (!search_timer_.isActive())
        search_timer_.start(1000);
}

searchServerAspect::~searchServerAspect(){
    search_timer_.stop();
}

static rea::regPip<QQmlApplicationEngine*> reg_tcp_linker([](rea::stream<QQmlApplicationEngine*>* aInput){
    std::cout << "init ssdp" << std::endl;
    static searchServerAspect search_server(aInput->scope()->data<rea::normalClient*>("client"));
    aInput->out();
}, QJsonObject(), "install0_tcp");

static rea::regPip<QQmlApplicationEngine*> load_window([](rea::stream<QQmlApplicationEngine*>* aInput){
    aInput->data()->load("file:gui/service/dpst_train/dialog/ViewMap.qml");
    aInput->data()->load("file:gui/service/dpst_train/dialog/VisibleSet.qml");
    aInput->out();
},  rea::Json("before", "loadMain"));
