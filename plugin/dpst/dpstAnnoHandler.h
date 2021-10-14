#include "handler.h"

class dpstAnnoHandler;

class dpstMode{
public:
    dpstMode(dpstAnnoHandler* aHandler){
        m_handler = aHandler;
    }
    virtual ~dpstMode(){}
    virtual QJsonObject getAnnos(const QString& aID, const QJsonObject& aModel, rea::stream<QString>* aInput) = 0;
    virtual void saveWorkFile(const QString& aID, rea::stream<QString>* aInput) = 0;
    virtual QString name() = 0;
protected:
    dpstAnnoHandler* m_handler;
};

class annoMode : public dpstMode{
public:
    annoMode(dpstAnnoHandler* aHandler) : dpstMode(aHandler){

    }
    QJsonObject getAnnos(const QString& aID, const QJsonObject& aModel, rea::stream<QString>* aInput) override;
    void saveWorkFile(const QString& aID, rea::stream<QString>* aInput) override;
    QString name() override{
        return "anno";
    }
};

class roiMode : public dpstMode{
public:
    roiMode(dpstAnnoHandler* aHandler) : dpstMode(aHandler){

    }
    QJsonObject getAnnos(const QString& aID, const QJsonObject& aModel, rea::stream<QString>* aInput) override;
    void saveWorkFile(const QString& aID, rea::stream<QString>* aInput) override;
    QString name() override{
        return "roi";
    }
};

class dpstAnnoHandler : private handler{
private:
    QString getSuffix() override{
        return "dpst_anno;tr_dpst_anno";
    }
    friend annoMode;
    friend roiMode;
    QHash<QString, std::shared_ptr<dpstMode>> m_mode;
public:
    dpstAnnoHandler();
private:
    std::shared_ptr<dpstMode> getMode(const QString& aID, rea::stream<QString>* aInput);
    void switchMode(const QString& aID, const QString& aMode);
    std::shared_ptr<rea::stream<QString>> getTaskInfo(const QString& aID, rea::stream<QString>* aInput);
    QHash<QString, long long> m_last_modified_time;  //avoid multi-monitors to open multi-times for the same modification
    QHash<QString, QJsonObject> m_data;
private:
    QString m_job_root;
    QString m_job_path = "";
    QJsonObject m_job_config;
};
