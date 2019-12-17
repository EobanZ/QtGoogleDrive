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
    void Authorize();
    virtual void UploadFiles(QStringList filePaths, QString folderName) = 0; //Should search for foler. If not found create new and upload files there
    virtual void CreateFolder(QString folderName, QString parent) = 0; //Should search for parent folder and create a new folder as child. If not found create folder in App root folder
    virtual void DeleteFolderWithFiles(QString folder) = 0; //Should find a folder and delete it with all the files folders it contains
    virtual void DeleteSingleFile(QString file) = 0; //Should delete a single file

private:

protected:
    QString GetContentTypeByExtension(QString file);


public:



    //TODO:
//    virtual void ListAllFiles();//ImmoCloud Root folder
//    virtual void ListAllFiles(QString folderId);
//    virtual void ListAllFolders();
//    virtual void DeleteFile(QString QNameOrId);
//    virtual void GetFileMetadata(QString);
//    virtual void ShareFolder(QString);

protected:
    Authenticator* m_authenticator;

signals:
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
    virtual void UploadFiles(QStringList files, QString folder) override;
    virtual void CreateFolder(QString folderName, QString parent) override;
    virtual void DeleteFolderWithFiles(QString folder) override;
    virtual void DeleteSingleFile(QString file) override;

private:
    void ResumeUploadFile(QString filePath, QString sessionLink, qint64 continuePosition);
    void UploadFile(QString filePath, QString folderId);
    //TODO: maybe create query response type and make a bool GetAllFolders(queryType& outType)
    bool GetFolderId(QString name, QString& id);
    bool GetFileId(QString name, QString& id);

};

#endif // CLOUDINTERFACE_H
