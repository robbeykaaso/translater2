#include "reaRemote.h"
#include "client.h"
#include "ssdp.h"
#include <QQmlApplicationEngine>
#include <QTimer>

class searchServerAspect{
public:
    searchServerAspect(rea2::normalClient* aClient);
    ~searchServerAspect();
private:
    void tryConnectServer();
    rea2::DiscoveryManager ssdp_;
    QTimer search_timer_;
};

searchServerAspect::searchServerAspect(rea2::normalClient* aClient){
    QObject::connect(&ssdp_, SIGNAL(FoundServer(QString, QString, QString)), aClient, SLOT(ServerFound(QString, QString, QString)));
    QObject::connect(&search_timer_, &QTimer::timeout, [this](){
        ssdp_.StartDiscovery();
        rea2::pipeline::instance()->run<QJsonObject>("clientBoardcast", rea2::Json("value", "search server..."));
    });

    rea2::pipeline::instance()->add<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
        if (aInput->data().empty())
            tryConnectServer();
    }, rea2::Json("after", "tryLinkServer"));

    rea2::pipeline::instance()->find("clientBoardcast")->nextF<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
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

static rea2::regPip<QQmlApplicationEngine*> reg_tcp_linker([](rea2::stream<QQmlApplicationEngine*>* aInput){
    std::cout << "init ssdp" << std::endl;
    auto clt = aInput->scope()->data<rea2::normalClient*>("client");
    if (clt)
        static searchServerAspect search_server(clt);
    aInput->out();
}, QJsonObject(), "install0_tcp");

static rea2::regPip<QQmlApplicationEngine*> load_dialog([](rea2::stream<QQmlApplicationEngine*>* aInput){
    aInput->data()->load("file:nwlan_ui/gui/service/dpst_train/dialog/ViewMap.qml");
    aInput->data()->load("file:nwlan_ui/gui/service/dpst_train/dialog/VisibleSet.qml");
    aInput->out();
},  rea2::Json("before", "loadMain"));
