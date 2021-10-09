#include "awsStorage.h"
#include "rea.h"
#include <QQmlApplicationEngine>
#include <QJsonDocument>
#include <QDateTime>

class awsStorage2 : public awsStorage{
public:
    awsStorage2(const QString& aRoot = "", const QJsonObject& aConfig = QJsonObject()) : awsStorage(aRoot, aConfig){}
    void initialize() override{
        rea::pipeline::instance()->add<bool, rea::pipePartial>([this](rea::stream<bool>* aInput) {
            QJsonObject dt;
            auto ret = readJsonObject(aInput->scope()->data<QString>("path"), dt);
            aInput->scope()->cache("data", dt);
            aInput->setData(ret)->out();
        }, rea::Json("name", m_root + "readJsonObject2"));
    }
};

using namespace rea;
static regPip<QQmlApplicationEngine*> test_stg([](stream<QQmlApplicationEngine*>*){
    static awsStorage aws_storage("testminio", rea::Json("local", true));
    aws_storage.initialize();
    static awsStorage2 aws_storage2("testminio", rea::Json("local", true));
    aws_storage2.initialize();
    static QString tag = "testAWSStorage";

    static int count0, count1;
    static int tm0, tm1;
    pipeline::instance()->add<QJsonObject>([](stream<QJsonObject>* aInput){
        auto stm = in(false, tag, std::make_shared<scopeCache>()
                           ->cache<QString>("path", "testFS.json")
                           ->cache("data", Json("hello", "world2")))
                   ->asyncCall("testminiowriteJsonObject")
                   ->asyncCall("testminioreadJsonObject");
        if (!stm->data())
            throw "aws_read/writeJsonObject error";
        auto ret = stm->scope()->data<QJsonObject>("data");
        if (ret.value("hello") != "world2")
            throw "aws_read/writeJsonObject error";

        stm->scope(true)->cache<QString>("path", "testFS.json")->cache("data", QJsonDocument(Json("hello", "world")).toJson());
        stm = stm->asyncCall("testminiowriteByteArray")->asyncCall("testminioreadByteArray");
        if (!stm->data())
            throw "aws_read/writeByteArray error";
        auto ret2 = stm->scope()->data<QByteArray>("data");
        if (QJsonDocument::fromJson(ret2).object().value("hello") != "world")
            throw "aws_read/writeByteArray error";

        stm->scope(true)->cache<QString>("path", "testDir/")->cache("data", QByteArray());
        stm = stm->asyncCall("testminiowriteByteArray");
        if (stm->data())
            throw "aws_writeByteArray error";

        auto stm2 = in<QString>("testDir", tag)->asyncCall("testminiolistFiles");
        auto ret3 = stm2->scope()->data<std::vector<QString>>("data");
        if (ret3.size() != 1)
            throw "aws_listFiles error";

        stm2->asyncCall("testminiodeletePath");
        in<QString>("testFS.json", tag)->asyncCall("testminiodeletePath");

        auto fls = in<QString>("test_storage", tag)->asyncCall("listFiles")->scope()->data<std::vector<QString>>("data");

        tm0 = QDateTime::currentDateTime().toTime_t();
        count0 = (fls.size() - 2) * 5;
        for (int i = 0; i < 5; ++i)
            for (auto i : fls)
                if (i != "." && i != ".."){
                    aInput->outs(false, "testminioreadJsonObject", tag + "2")->scope(true)->cache<QString>("path", "test_storage/" + i);
                }

/*        tm1 = QDateTime::currentDateTime().toTime_t();
        count1 = (fls.size() - 2) * 5;
        for (int i = 0; i < 5; ++i)
            for (auto i : fls)
                if (i != "." && i != "..")
                    aInput->outs(false, "testminioreadJsonObject2", tag + "2")->scope(true)->cache<QString>("path", "test_storage/" + i);
*/
    //    aInput->outs(true);
    }, Json("name", tag, "external", "js"));

    //prove reading of file system by multithreads
    rea::pipeline::instance()->find("readJsonObject")->nextF<bool>([](rea::stream<bool>* aInput){
        aInput->asyncCall("testminiowriteJsonObject");
    }, tag + "3");

    rea::pipeline::instance()->find("testminioreadJsonObject")->nextF<bool>([](rea::stream<bool>* aInput){
        auto dt = aInput->scope()->data<QJsonObject>("data");
        count0--;
        std::cout << count0 << std::endl;
        if (!count0){
            std::cout << QDateTime::currentDateTime().toTime_t() - tm0 << std::endl;
        }
    }, tag + "2");

    rea::pipeline::instance()->find("testminioreadJsonObject2")->nextF<bool>([](rea::stream<bool>* aInput){
        auto dt = aInput->scope()->data<QJsonObject>("data");
        count1--;
        std::cout << count1 << std::endl;
        if (!count1){
            std::cout << QDateTime::currentDateTime().toTime_t() - tm1 << std::endl;
        }
    }, tag + "2");
}, QJsonObject(), "initRea");
