#include "rea.h"
#include <QSettings>

class recoverUserState{
public:
    recoverUserState(){
        m_sets = std::make_shared<QSettings>("dpst", "tr");

        /*rea::pipeline::instance()->add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            m_sets->setValue("storage", aInput->data());
        }, rea::Json("name", "storageOpened"));*/

        rea::pipeline::instance()->find("c++_layoutChanged")->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            if (m_recovering)
                m_recovering = false;
            else
                m_sets->setValue("layout", aInput->data());
        });

        rea::pipeline::instance()->add<QString>([this](rea::stream<QString>* aInput){
            /*auto stg = m_sets->value("storage").toJsonObject();
            if (stg.value("mode") == "file_sys")
                aInput->outs(stg.value("root").toString(), "_Open_Folder");
            else if (stg.value("mode") == "aws_sys"){
                aInput->outs(stg.value("config").toObject(), "_Open_AWS");
            }*/

            auto layout = m_sets->value("layout").toJsonArray();
            if (layout.empty())
                layout.push_back(rea::Json("i", "0", "x", 0, "y", 0, "w", 1, "h", 2, "dely", 0));
            m_recovering = true;
            aInput->outs(layout, "loadView");
        }, rea::Json("name", "reaGridLoaded"));
    }
private:
    std::shared_ptr<QSettings> m_sets;
    bool m_recovering = false;
};

static recoverUserState recv_stat;
