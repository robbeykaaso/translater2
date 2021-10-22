#include "rea.h"
#include <QSettings>

class recoverUserState{
public:
    recoverUserState(){
        m_sets = std::make_shared<QSettings>("dpst", "tr");

        rea2::pipeline::instance()->find("saveGridModel")->nextF<QJsonObject>([this](rea2::stream<QJsonObject>* aInput){
            if (m_recovering){
                std::cout << "recover layout finished" << std::endl;
                m_recovering = false;
            }else{
                //qDebug() << aInput->data();
                m_sets->setValue("layout", aInput->data());
            }
        });

        rea2::pipeline::instance()->add<QString>([this](rea2::stream<QString>* aInput){
            auto layout_cfg = m_sets->value("layout").toJsonObject();
            //qDebug() << layout_cfg;
            auto layout = layout_cfg.value("layout").toArray();
            if (layout.empty()){
                layout.push_back(rea2::Json("i", "0", "x", 0, "y", 0, "w", 1, "h", 2, "dely", 0));
                layout_cfg.insert("layout", layout);
            }
            std::cout << "recover layout start" << std::endl;
            m_recovering = true;
            aInput->outs(layout_cfg, "loadView");
        }, rea2::Json("name", "reaGridLoaded"));

        rea2::pipeline::instance()->add<QString>([](rea2::stream<QString>* aInput){
            aInput->out();
        }, rea2::Json("name", "viewLoaded"))->nextFB<QString>([this](rea2::stream<QString>* aInput){
            auto layout_cfg = aInput->scope()->data<QJsonObject>("layout");
            if (layout_cfg.empty())
                layout_cfg = m_sets->value("layout").toJsonObject();
            auto tp = layout_cfg.value("ide_type").toObject();

            auto dt = aInput->data();
            auto idx = dt.split("_")[0].split("reagrid")[1];
            if (tp.value(idx) == "resource__"){
                auto sts = layout_cfg.value("ide_status").toObject();
                auto stg = sts.value(idx).toObject();
                if (stg.value("mode") == "file_sys")
                    aInput->outs(stg.value("root").toString(), "openFolder_" + aInput->data());
                else if (stg.value("mode") == "aws_sys"){
                    aInput->outs(stg.value("config").toObject(), "openAWS_" + aInput->data());
                }
            }
        }, "", rea2::Json("name", "reaResourceLoaded"));
    }
private:
    std::shared_ptr<QSettings> m_sets;
    bool m_recovering = false;
};

static recoverUserState recv_stat;
