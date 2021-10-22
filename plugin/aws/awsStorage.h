#ifndef REAL_FRAMEWORK_AWSSTORAGE_H_
#define REAL_FRAMEWORK_AWSSTORAGE_H_

#include "aws_s3.h"
#include "storage.h"
#ifdef USEOPENCV
    #include "opencv2/opencv.hpp"
#endif

class awsStorage : public rea2::fsStorage {
public:
    awsStorage(const QString& aType = "");
    bool isValid() override { return m_aws.isValid(); }
    void initialize() override;
    void initializeAWSConfig(const QJsonObject& aConfig);
protected:
    std::vector<QString> listFiles(const QString& aDirectory) override;
    bool writeJsonObject(const QString& aPath, const QJsonObject& aData) override;
    bool writeByteArray(const QString& aPath, const QByteArray& aData) override;
    bool readJsonObject(const QString& aPath, QJsonObject& aData) override;
    bool readByteArray(const QString& aPath, QByteArray& aData) override;
    void deletePath(const QString& aPath) override;
    long long lastModifiedTime(const QString& aPath) override;
#ifdef USEOPENCV
    bool writeImage(const QString&, const QImage&) override;
    bool readImage(const QString&, QImage&) override;
    bool writeCVMat(const QString& aPath, const cv::Mat& aData);
    bool readCVMat(const QString& aPath, cv::Mat& aData);
#endif
protected:
    AWSClient m_aws;
private:
    void checkSameEnd(const QJsonObject& aConfig);
    void deleteAWSDirectory(AWSClient& aClient, const QString& aBucket, const QString& aPath);
};

#endif
