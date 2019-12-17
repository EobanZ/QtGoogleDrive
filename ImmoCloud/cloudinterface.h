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
    void TestUploadResumable();
    virtual void CreateFolder(QString folderName, QString parentId) = 0;


private:


protected:
    QString GetContentTypeByExtension(QString file);
    virtual void UploadFile(QString filePath, QString folderId) = 0;//Start Upload
    virtual void ResumeUploadFile(QString filePath, QString sessionLink, qint64 continuePosition) = 0;


    //TODO:
//    virtual void CreateFolder(QString name);
//    virtual void ListAllFiles();//ImmoCloud Root folder
//    virtual void ListAllFiles(QString folderId);
//    virtual void ListAllFolders();
//    virtual void LookupFolderIDwithPath();
//    virtual void DeleteFile(QString QNameOrId);
//    virtual void GetFileMetadata(QString);
//    virtual void ShareFolder(QString);
//    virtual void UploadFile(QString file, QString folderId);

protected:
    Authenticator* m_authenticator;
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_networkReply;
    QString m_currentFile;
    QString m_currentUploadUrl;
    bool m_isUploading;


signals:
    void OnUploadLinkReceived(QString url, QString filePath);

public slots:
};



///////////////
struct GoogleConfig{
    QString clientID;
    QString projectID;
    QString authUri;
    QString tokenUri;
    QString authProvider;
    QString clientSecret;
    QString redirectUri;
};

class CloudInterface_GoogleDrive : public CloudInterface
{
public:
    CloudInterface_GoogleDrive(Authenticator_GoogleDrive* auth, QObject *parent = nullptr);
    CloudInterface_GoogleDrive(QString clientID, QString cliedSecret, QObject *parent = nullptr);
    virtual void UploadFiles(QStringList files) override;
    virtual void UploadFile(QString filePath, QString folderId) override;
    virtual void ResumeUploadFile(QString filePath, QString sessionLink, qint64 continuePosition) override;
    virtual void CreateFolder(QString folderName, QString parentId) override;

};

#endif // CLOUDINTERFACE_H
