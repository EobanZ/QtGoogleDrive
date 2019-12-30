#ifndef CLOUDINTERFACE_H
#define CLOUDINTERFACE_H

#include <QObject>
#include <QMap>
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
    virtual QString CreateFolder(QString folderName, QString parentId) = 0; //Should search for parent folder and create a new folder as child. If not found create folder in App root folder
    virtual QList<QPair<QString,QString>> GetAllChildFolders(QString folderName) = 0; //Should return all the names and ids of the children folder.
    virtual QString MakeOrGetShareLink(QString fileId) = 0;
    virtual void DeleteFolder(QString folder, bool isId) = 0;

private:

protected:
    QString GetContentTypeByExtension(QString file);


public:



    //TODO:
//    virtual void ListAllFolders();
//    virtual void DeleteFile(QString QNameOrId);
//    virtual void ShareFolder(QString);

protected:
    Authenticator* m_authenticator;

signals:
    void IsReady();
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
    virtual QString CreateFolder(QString folderName, QString parent) override;
    virtual QList<QPair<QString,QString>> GetAllChildFolders(QString folderName) override;
    virtual QString MakeOrGetShareLink(QString fileId) override;
    virtual void DeleteFolder(QString folder, bool isId) override;

private:
    void ResumeUploadFile(QString filePath, QString sessionLink, qint64 continuePosition);
    void UploadFile(QString filePath, QString folderId);
    void UpdateFoldersSnapshot();

    //TODO: QMap<QString, QString> GetAllFoldersInFolder(QString folderId);

private:
    QMap<QString, QString> m_foldersSnapshot;



};

#endif // CLOUDINTERFACE_H
