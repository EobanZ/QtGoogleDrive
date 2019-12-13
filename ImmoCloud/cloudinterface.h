#ifndef CLOUDINTERFACE_H
#define CLOUDINTERFACE_H

#include <QObject>
class Authenticator;
class Authenticator_GoogleDrive;
class QNetworkAccessManager;
class QNetworkReply;

class CloudInterface : public QObject
{
    Q_OBJECT
public:
    explicit CloudInterface(Authenticator* auth,QObject *parent = nullptr);
    CloudInterface(QString clientID, QString ClientSecret, QObject* parent = nullptr);
    virtual void UploadFiles(QStringList filePaths) = 0;
    void Authorize();
    void TestUploadMultiPart();
    void TestUploadResumable();


private:


protected:
    QString GetContentTypeByExtension(QString file);
    virtual void UploadFile(QString filePath) = 0;

protected:
    Authenticator* m_authenticator;
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_networkReply;
    QString m_currentFile;

signals:
    void OnUploadLinkReceived(QString url, QString filePath);

public slots:
};


///////////////
class CloudInterface_GoogleDrive : public CloudInterface
{
public:
    CloudInterface_GoogleDrive(Authenticator_GoogleDrive* auth, QObject *parent = nullptr);
    CloudInterface_GoogleDrive(QString clientID, QString cliedSecret, QObject *parent = nullptr);
    virtual void UploadFiles(QStringList files) override;
    virtual void UploadFile(QString filePath) override;
    void HandleCreateUploadSessionReply(QNetworkReply* reply);
    void HandleUploadingReply(QNetworkReply* reply);
};

#endif // CLOUDINTERFACE_H
