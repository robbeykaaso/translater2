#include "rea.h"
#include "awsStorage.h"
#include <QJsonDocument>
#include <QDir>

#include <Windows.h>
//
QJsonObject checkMinIO(){
    // 打开服务管理对象
    SC_HANDLE hSC = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    if( hSC == NULL){
        return QJsonObject();
    }

    /*LPENUM_SERVICE_STATUS lpServices    = NULL;
    DWORD    nSize = 0;
    DWORD    n;
    DWORD    nResumeHandle = 0;

    lpServices = (LPENUM_SERVICE_STATUS) LocalAlloc(LPTR, 64 * 1024);      //注意分配足够的空间
    EnumServicesStatus(hSC,SERVICE_WIN32,
                       SERVICE_STATE_ALL,
                       (LPENUM_SERVICE_STATUS)lpServices,
                       64 * 1024,
                       &nSize,
                       &n,
                       &nResumeHandle);
    for (int i = 0; i < n; i++)
    {
        auto test = lpServices[i];
        if (QString::fromStdString(test.lpServiceName) == "MinIO")
            std::cout << test.lpServiceName << std::endl;
    }*/

    SC_HANDLE hSvc = ::OpenService( hSC, "MinIO", SERVICE_QUERY_STATUS);

    //auto tmp = GetLastError();

    if( hSvc == NULL)
    {
        ::CloseServiceHandle( hSC);
        return QJsonObject();
    }
    // 获得服务的状态
    SERVICE_STATUS status;
    if( ::QueryServiceStatus( hSvc, &status) == FALSE)
    {
        ::CloseServiceHandle( hSvc);
        ::CloseServiceHandle( hSC);
        return QJsonObject();
    }else{
        QJsonObject ret;
        ret.insert("running", status.dwCurrentState == SERVICE_RUNNING);
        ::CloseServiceHandle( hSvc);
        ::CloseServiceHandle( hSC);
        return ret;
    }
    //如果处于停止状态则启动服务，否则停止服务。
    /*    if( status.dwCurrentState == SERVICE_RUNNING)
    {
        // 停止服务
        if( ::ControlService( hSvc,
                             SERVICE_CONTROL_STOP, &status) == FALSE)
        {
            ::CloseServiceHandle( hSvc);
            ::CloseServiceHandle( hSC);
            return;
        }
        // 等待服务停止
        while( ::QueryServiceStatus( hSvc, &status) == TRUE)
        {
            ::Sleep( status.dwWaitHint);
            if( status.dwCurrentState == SERVICE_STOPPED)
            {
                ::CloseServiceHandle( hSvc);
                ::CloseServiceHandle( hSC);
                return;
            }
        }
    }
    else if( status.dwCurrentState == SERVICE_STOPPED)
    {
        // 启动服务
        if( ::StartService( hSvc, NULL, NULL) == FALSE)
        {
            ::CloseServiceHandle( hSvc);
            ::CloseServiceHandle( hSC);
            return;
        }
        // 等待服务启动
        while( ::QueryServiceStatus( hSvc, &status) == TRUE)
        {
            ::Sleep( status.dwWaitHint);
            if( status.dwCurrentState == SERVICE_RUNNING)
            {
                ::CloseServiceHandle( hSvc);
                ::CloseServiceHandle( hSC);
                return;
            }
        }
    }*/
}

void awsStorage::checkSameEnd(const QJsonObject& aConfig){
    if (!aConfig.empty())
        initializeAWSConfig(aConfig);
}

void awsStorage::initialize(){
#define READSTORAGE(aType) \
    rea::pipeline::instance()->add<bool, rea::pipeParallel>([this](rea::stream<bool>* aInput) { \
        checkSameEnd(aInput->scope()->data<QJsonObject>("config")); \
        Q##aType dt; \
        auto ret = read##aType(aInput->scope()->data<QString>("path"), dt); \
        aInput->scope()->cache("data", dt); \
        aInput->setData(ret)->out(); \
    }, rea::Json("name", m_root + STR(read##aType)))

#define WRITESTORAGE(aType) \
    rea::pipeline::instance()->add<bool, rea::pipeParallel>([this](rea::stream<bool>* aInput){ \
        checkSameEnd(aInput->scope()->data<QJsonObject>("config")); \
        aInput->setData(write##aType(aInput->scope()->data<QString>("path"), aInput->scope()->data<Q##aType>("data")))->out(); \
}, rea::Json("name", m_root + STR(write##aType)))

#ifdef USEOPENCV
    rea::pipeline::instance()->add<bool, rea::pipeParallel>([this](rea::stream<bool>* aInput) {
        checkSameEnd(aInput->scope()->data<QJsonObject>("config"));
        cv::Mat dt;
        auto ret = readCVMat(aInput->scope()->data<QString>("path"), dt);
        aInput->scope()->cache("data", dt);
        aInput->setData(ret)->out();
    }, rea::Json("name", m_root + "readCVMat"));

    rea::pipeline::instance()->add<bool, rea::pipeParallel>([this](rea::stream<bool>* aInput){
        checkSameEnd(aInput->scope()->data<QJsonObject>("config"));
        aInput->setData(writeCVMat(aInput->scope()->data<QString>("path"), aInput->scope()->data<cv::Mat>("data")))->out();
    }, rea::Json("name", m_root + "writeCVMat"));
#endif

    READSTORAGE(JsonObject);
    READSTORAGE(ByteArray);
    READSTORAGE(Image);
    WRITESTORAGE(JsonObject);
    WRITESTORAGE(ByteArray);
    WRITESTORAGE(Image);

    rea::pipeline::instance()->add<QString, rea::pipePartial>([this](rea::stream<QString>* aInput){
        checkSameEnd(aInput->scope()->data<QJsonObject>("config"));
        auto fls = listFiles(aInput->data());
        aInput->scope()->cache("data", fls);
        aInput->out();
    }, rea::Json("name", m_root + "listFiles"));

    rea::pipeline::instance()->add<QString, rea::pipePartial>([this](rea::stream<QString>* aInput){
        checkSameEnd(aInput->scope()->data<QJsonObject>("config"));
        std::vector<QString> fls;
        listAllFiles(aInput->data(), fls);
        aInput->scope()->cache("data", fls);
        aInput->out();
    }, rea::Json("name", m_root + "listAllFiles"));

    rea::pipeline::instance()->add<QString, rea::pipeParallel>([this](rea::stream<QString>* aInput){
        checkSameEnd(aInput->scope()->data<QJsonObject>("config"));
        deletePath(aInput->data());
        aInput->out();
    }, rea::Json("name", m_root + "deletePath"));

    rea::pipeline::instance()->add<QString, rea::pipePartial>([this](rea::stream<QString>* aInput){
        checkSameEnd(aInput->scope()->data<QJsonObject>("config"));
        aInput->outs(lastModifiedTime(aInput->data()));
    }, rea::Json("name", m_root + "lastModified"));
}

awsStorage::awsStorage(const QString& aType) : fsStorage(aType){
}

void awsStorage::initializeAWSConfig(const QJsonObject& aConfig){
    auto rt = aConfig.value("root").toString();
    bool new_rt = false;
    if (rt != m_root){
        m_root = rt;
        new_rt = true;
    }
    m_aws.initialize([](){
        bool ret = false;
        auto sts = checkMinIO();
        do{
            if (sts.contains("running"))
                ret = true;
            if (!sts.value("running").toBool()){
                auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_start.bat\"";
                //system("start /b minio.exe server storage");
                system(cmd.toStdString().c_str());
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            }else
                break;
            auto cmd = "\"" + QDir::currentPath() + "/minIO/storage_start.bat\"";
            //system("start /b minio.exe server storage");
            system(cmd.toStdString().c_str());
        }while(0);
        return ret;
    }, aConfig);
    //std::cout << m_root.toStdString() << std::endl;
    if (new_rt){
        auto rt = m_root.split("/")[0];
        m_aws.create_bucket(rt.toStdString().c_str());
    }
}

std::vector<QString> awsStorage::listFiles(const QString& aDirectory){
    std::vector<std::string> lst;
    QString dir = "";
    if (aDirectory != "")
        dir = aDirectory + "/";
    QString mk = "";
    std::vector<QString> ret;
    do{
        m_aws.list_s3_objects(m_root.toStdString().data(), dir.toStdString().data(), lst, mk.toStdString().data());
        for (auto i : lst){
            auto fl = QString::fromStdString(i);
            fl.remove(0, aDirectory.length() + (aDirectory == "" ? 0 : 1));
            if (fl.contains("/")){
                fl.truncate(fl.indexOf("/"));
                ret.push_back(fl);
            }else{
                ret.push_back(fl);
            }
        }
        if (lst.size() >= 999){
            mk = QString::fromStdString(lst.back());
            lst.clear();
        }else
            mk = "";
    }while(mk != "");
    return ret;
}

bool awsStorage::writeJsonObject(const QString& aPath, const QJsonObject& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");

    QJsonDocument doc(aData);
    auto str = doc.toJson().toStdString();
    stm->write(str.c_str(), int(str.size()));

    return m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

#ifdef USEOPENCV

#include "plugin/opencv/util.h"

bool awsStorage::writeCVMat(const QString& aPath, const cv::Mat& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");
    //https://blog.csdn.net/weixin_42112458/article/details/83117305?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task
    auto suffix = aPath.mid(aPath.lastIndexOf("."), aPath.length());
    std::vector<uchar> buff;
    cv::imencode(suffix.toStdString().data(), aData, buff);
    char* dt = new char[buff.size()];
    memcpy(dt, reinterpret_cast<char*>(&buff[0]), buff.size());
    stm->write(dt, int(buff.size()));
    delete[] dt;
    return m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
}

bool awsStorage::readCVMat(const QString& aPath, cv::Mat& aData){
    int sz;
    auto img = m_aws.get_s3_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), sz);
    if (img != nullptr){
        std::string str2(img, size_t(sz));
        std::vector<char> vec(str2.c_str(), str2.c_str() + str2.size());
        aData = cv::imdecode(vec, cv::IMREAD_UNCHANGED);
        return true;
    }
    return false;
}

bool awsStorage::writeImage(const QString& aPath, const QImage& aData){
    return writeCVMat(aPath, QImage2cvMat(aData));
}

bool awsStorage::readImage(const QString& aPath, QImage& aData){
    cv::Mat dt;
    auto ret = readCVMat(aPath, dt);
    aData = cvMat2QImage(dt);
    return ret;
}
#endif

bool awsStorage::writeByteArray(const QString& aPath, const QByteArray& aData){
    auto stm = Aws::MakeShared<Aws::StringStream>("object");

    auto str = aData.toStdString();
    stm->write(str.c_str(), int(str.size()));

    auto ret = m_aws.put_s3_string_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), stm);
    if (aPath.endsWith("/"))
        return false;
    else
        return ret;
}

bool awsStorage::readJsonObject(const QString& aPath, QJsonObject& aData){
    int sz;
    auto str = m_aws.get_s3_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), sz);
    if (str != nullptr){
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(str, sz).toUtf8());
        aData = doc.object();
        delete[]str;
        return true;
    }
    return false;
}

bool awsStorage::readByteArray(const QString& aPath, QByteArray& aData){
    int sz;
    auto str = m_aws.get_s3_object(m_root.toStdString().c_str(), aPath.toStdString().c_str(), sz);
    if (str != nullptr){
        aData = QByteArray(str, sz);
        delete[]str;
        return true;
    }
    return false;
}

void deleteAWSDirectory(AWSClient& aClient, const QString& aBucket, const QString& aPath){
    if (aPath.indexOf(".") >= 0 || aPath.endsWith("/"))
        aClient.delete_s3_object(aBucket.toStdString().c_str(), aPath.toStdString().c_str());
    else{
        std::vector<std::string> lst;
        aClient.list_s3_objects(aBucket.toStdString().c_str(), aPath.toStdString().c_str(), lst);
        for (auto i : lst)
            deleteAWSDirectory(aClient, aBucket, QString::fromStdString(i));
    }
}

void awsStorage::deletePath(const QString& aPath){
    deleteAWSDirectory(m_aws, m_root, aPath);
}

long long awsStorage::lastModifiedTime(const QString& aPath){
    return m_aws.last_s3_modifiedTime(m_root.toStdString().c_str(), aPath.toStdString().c_str());
}
