#include "cloudinterface.h"
#include "authenticator.h"

#include "windows.h"
#include <qnetworkreply.h>

#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QFile>
#include <QEventLoop>
#include <QProgressDialog>

CloudInterface::CloudInterface(Authenticator* auth,QObject *parent): QObject(parent), m_authenticator(auth)
{
    connect(m_authenticator, &Authenticator::OnSuccess, [&](){emit IsReady();});
}

void CloudInterface::Authorize(){
    if(m_authenticator)
        m_authenticator->StartAuth();
}

QString CloudInterface::GetContentTypeByExtension(QString extension)
{
    QString contentType("");
    extension = extension.toLower();

    if(extension == "doc" || extension == "docx") contentType = "application/msword";
    if(extension == "xls") contentType = "application/vnd.ms-excel";
    if(extension == "ppt" || extension == "pptx") contentType = "application/vnd.ms-powerpoint";
    if(extension == "pdf") contentType = "application/pdf";
    if(extension == "exe") contentType = "application/x-msdos-program";
    if(extension == "rar") contentType = "application/rar";
    if(extension == "png") contentType = "image/png";
    if(extension == "png") contentType = "application/rtf";
    if(extension == "tar") contentType = "application/x-tar";
    if(extension == "zip") contentType = "application/zip";
    if(extension == "") contentType = "";
    if(extension == "jpeg" || extension == "jpg" || extension == "jpe") contentType = "image/jpeg";
    if(extension == "gif") contentType = "image/gif";
    if(extension == "wav") contentType = "application/x-wav";
    if(extension == "tiff" || extension == "tif") contentType = "image/tiff";
    if(extension == "txt" || extension == "cpp" || extension == "h" || extension == "c") contentType = "text/plain";
    if(extension == "mpeg" || extension == "mpg" || extension == "mpe" ) contentType = "video/mpeg";
    if(extension == "qt" || extension == "mov") contentType = "video/quicktime";
    if(extension == "qvi") contentType = "video/x-msvideo";
    if(extension == "video/x-sgi-movie") contentType = "movie";
    if(extension == "exe") contentType = "application/x-msdos-program";
    if(contentType == "") contentType = "application/octet-stream";

    return contentType;

}

CloudInterface_GoogleDrive::CloudInterface_GoogleDrive(Authenticator_GoogleDrive* auth, QObject *parent): CloudInterface((Authenticator*)auth, parent)
{


}

CloudInterface_GoogleDrive::CloudInterface_GoogleDrive(QString clientID, QString clientSecret, QObject *parent) : CloudInterface(new Authenticator_GoogleDrive(clientID, clientSecret, parent), parent)
{

}

void CloudInterface_GoogleDrive::UploadFiles(QStringList files, QString folder)
{

    QString fId;
    QString fName;
    if(folder != "root")
    {
        //Check if the Name or Id already exitst
        UpdateFoldersSnapshot();
        m_foldersSnapshot[folder];
        if(fId.isEmpty())
        {
            for(auto e: m_foldersSnapshot.keys())
            {
                if(m_foldersSnapshot.value(e) == folder)
                {
                    fName = e;
                    fId = folder;
                }
            }
        }

        //If still empty, folder doesn't exists
        if(fId.isEmpty())
        {
            qDebug() << "UploadFiles() error. Folder doestn't exist:  " << fName << " " << fId;
            //TODO: emit abort
            return;
        }
    }

    //if here still empty and not return its the root folder
    if(fId.isEmpty())
        fId = folder;//"root"

    //Check if authorized.
    if(!m_authenticator->isGranted())
        return; //TODO: emit abort

    QProgressDialog prog("Uploading files", "cancel", 0, files.count(), nullptr);
    prog.setCancelButton(nullptr);
    prog.show();
    prog.setMinimumWidth(200);
    prog.setWindowModality(Qt::WindowModal);
    int i = 0;

    foreach (auto path, files) {
        UploadFile(path, fId);
        prog.setValue(++i);
    }

}

//Uploads a file to a specific folder. Use "root" for default folder
void CloudInterface_GoogleDrive::UploadFile(QString filePath, QString folder)
{
    static int uploadAttemptCount = 0;
    uploadAttemptCount++;

    //Check if authorized
    if(!m_authenticator->isGranted())
    {
        //TODO: emit abort signal
        qDebug() << "UploadFile() " << "Auth not granted";
        return;
    }

    //Check if file exists
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
    {
        //TODO: emit abort signal
        qDebug() << "UploadFile()" << " File doesn't exist";
        return;
    }

    //Create one NetworkAccessManager for all requests
    QNetworkAccessManager* nManager = new QNetworkAccessManager(this);

    //Create MetaData with infos of the file to upload
    QJsonObject root;
    root.insert("name", fileInfo.fileName());
    if(folder != "root")
    {
        QJsonArray parents;
        parents.push_back(folder);
        root.insert("parents", QJsonValue(parents));
    }
    QJsonDocument body(root);
    QByteArray data = body.toJson();

    //Init Upload Session
    QNetworkRequest request;
    request.setUrl(QUrl("https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable"));
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());
    request.setRawHeader("Content-Lenght", QByteArray::number(data.size()));
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("X-Upload-Content-Type", GetContentTypeByExtension(fileInfo.suffix()).toUtf8());
    request.setRawHeader("X-Upload-Content-Length", QByteArray::number(fileInfo.size()));

    QEventLoop loop;
    QNetworkReply* reply = nManager->post(request, data);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    reply->setParent(nManager); //so it will be destoryed with the NetworkAccessManager
    loop.exec();

    //Handle Session Creation Reply
    //Check for error
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        //Handle Error
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = document.object();
        QJsonObject error = root["error"].toObject();
        QJsonValue code = error.value("code");
        QJsonValue message = error.value("message");
        qDebug() << "code: " + QString::number(code.toInt()) << "  message: " << message.toString();

        nManager->deleteLater();
        //TODO: Emit Upload abort
        return;
    }

    //If session was sucessfully created -> Parse reply
    QString Content_Lenght = QString::fromUtf8(reply->rawHeader("Content-Lenght"));
    QString Content_Type = QString::fromUtf8(reply->rawHeader("Content-Type"));
    QString X_GUploader_UploadID = QString::fromUtf8(reply->rawHeader("X-GUploader-UploadID"));
    QString Location = QString::fromUtf8(reply->rawHeader("Location"));
    QString Cache_Control = QString::fromUtf8(reply->rawHeader("Cache-Control"));
    QString Date = QString::fromUtf8(reply->rawHeader("Date"));
    if(Location.isEmpty()){

        qDebug() << "UploadFile()" << " Location Uploadsession Link was empty";
        //Emit upload abort
        return;
    }


    /////////////////////////////////////////////////////

    //Start file upload
    QNetworkRequest uploadRequest(Location);
    QFile file(filePath);

    //Open and read file to upload
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "UploadFile()" << " Couldn't open file: " << filePath;
        //TODO: emit abort
        return;
    }

    //Set the file size and type in the request header
    uploadRequest.setRawHeader("Content-Lenght", QByteArray::number(file.size()));
    uploadRequest.setRawHeader("Content-Type", GetContentTypeByExtension(fileInfo.suffix()).toUtf8());

    //Loop untill reply is ready
    QEventLoop loop2;
    QNetworkReply* uploadReply = nManager->put(uploadRequest, &file);
    uploadReply->setParent(nManager);
    connect(uploadReply, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    loop2.exec();
    file.close();


    //Handle reply codes
    int status = uploadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 308)
    {
        qDebug() << "Upload didn't finish code: 308. Should be resumed";

        //Request upload status
        QNetworkRequest uploadStatusRequest(Location);
        uploadStatusRequest.setRawHeader("Content-Length", "0");
        uploadStatusRequest.setRawHeader("Content-Range", "bytes */*");

        QEventLoop loop3;
        QNetworkReply* replyUploadStatus = nManager->put(request, QByteArray());
        replyUploadStatus->setParent(nManager);
        connect(replyUploadStatus, &QNetworkReply::finished, &loop3, &QEventLoop::quit);
        loop3.exec();

        QString range = QString::fromUtf8(reply->rawHeader("Range"));
        auto s1 = range.split("=");
        auto receivedBytes = s1[1].split("-");
        nManager->deleteLater();
        ResumeUploadFile(filePath, Location ,receivedBytes[1].toUInt()+1);
        return;

    }

    if(status == 404)
    {
        qDebug() << "Upload code: 404 Not Found. Sould restart upload";
        nManager->deleteLater();
        if(uploadAttemptCount >= 8)
        {

            //TODO: emit abort
            return;
        }
        UploadFile(filePath, folder);
        uploadAttemptCount = 0; //When returning from recurcive funktion set count back to 0. Return when upload was successfull or it faile 8 times
        return;
    }

    //else successfull

    qDebug() << "upload completed. Code: " << uploadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() << reply->rawHeaderPairs();
    //qDebug() << uploadReply->readAll();

    nManager->deleteLater();
    return;
}

void CloudInterface_GoogleDrive::UpdateFoldersSnapshot()
{
    QNetworkAccessManager* nManager = new QNetworkAccessManager();
    QUrl url("https://www.googleapis.com/drive/v3/files");
    QUrlQuery query;
    //query.addQueryItem("access_token=", m_authenticator->GetToken().toUtf8());
    query.addQueryItem("pageSize", "100");
    query.addQueryItem("q", "mimeType='application/vnd.google-apps.folder'");
    //query.addQueryItem("q", "trashed=false");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());

    QEventLoop loop;
    QNetworkReply* reply = nManager->get(request);
    reply->setParent(nManager);
    connect(nManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    loop.exec();

    m_foldersSnapshot.clear();

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject root = doc.object();
    QJsonArray files = root["files"].toArray();
    int filesCount = files.size();
    for(int i = 0; i < filesCount; i++)
    {
        QString name = files[i].toObject().value("name").toString();
        QString id = files[i].toObject().value("id").toString();
        m_foldersSnapshot[name] = id;
    }

    //qDebug() << doc;
    nManager->deleteLater();
}

void CloudInterface_GoogleDrive::ResumeUploadFile(QString filePath, QString sessionLink, qint64 continuePosition)
{
    static int resumeUploadAttempts = 0;
    resumeUploadAttempts++;

    QFileInfo fileInfo(filePath);

    QNetworkAccessManager* nManager = new QNetworkAccessManager();
    QNetworkRequest request(sessionLink);
    request.setRawHeader("Content-Lenght", QByteArray::number(fileInfo.size()-continuePosition));
    request.setRawHeader("Content-Range", QString("bytes %1-%2/%3").arg(continuePosition).arg(fileInfo.size()-1).arg(fileInfo.size()).toUtf8());

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)){
        //TODO: emit abort
        return;
    }

    file.seek(continuePosition);
    QEventLoop loop;
    QNetworkReply* reply = nManager->put(request, file.read(fileInfo.size())); //TODO: test this
    connect(reply, &QNetworkReply::finished,&loop,&QEventLoop::quit);
    reply->setParent(nManager);
    loop.exec();

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 308)
    {
        //Resume again
        QString range = QString::fromUtf8(reply->rawHeader("Range"));
        auto s1 = range.split("=");
        auto receivedBytes = s1[1].split("-");
        if(resumeUploadAttempts > 20)
        {
            //TODO: emit upload abort
            nManager->deleteLater();
            resumeUploadAttempts = 0;
            return;
        }
        ResumeUploadFile(filePath, sessionLink ,receivedBytes[1].toUInt()+1);
    }


    nManager->deleteLater();
    return;

}

QString CloudInterface_GoogleDrive::CreateFolder(QString folderName, QString parentId)
{
    QString result = "";


    auto searchSnapshot = [&](){
        auto id = m_foldersSnapshot[folderName];
        if(!id.isEmpty())
        {
            result = id;
        }
    };

    searchSnapshot(); //1. Search the current snapshot
    if(!result.isEmpty())
        return result;

    UpdateFoldersSnapshot(); //2. if not found. Get fresh snapshot and search again
    searchSnapshot();
    if(!result.isEmpty())
        return result;

    //3. if still not found create new folder
    QNetworkAccessManager* nManager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart"));

    QHttpMultiPart multiPart(QHttpMultiPart::ContentType::RelatedType);
    multiPart.setBoundary("foo_bar_baz");
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());

    QJsonObject root;
    root.insert("name", folderName);
    root.insert("description", folderName);
    root.insert("mimeType", "application/vnd.google-apps.folder");
    if(parentId != "root")
    {
        QJsonArray parents;
        parents.push_back(parentId);
        root.insert("parents", QJsonValue(parents));
    }
    QJsonDocument doc(root);

    QHttpPart part1;
    part1.setRawHeader("Content-Type","application/json; charset=UTF-8");
    part1.setBody(doc.toJson());

    multiPart.append(part1);

    QEventLoop* loop = new QEventLoop;
    QNetworkReply* reply = nManager->post(request, &multiPart);
    reply->setParent(nManager);
    connect(reply, &QNetworkReply::finished, loop, &QEventLoop::quit);
    loop->exec();

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
    {
        QJsonObject replyRoot = QJsonDocument::fromJson(reply->readAll()).object();
        QString name = replyRoot.value("name").toString();
        QString id = replyRoot.value("id").toString();
        m_foldersSnapshot[name] = id;
        result = id;
    }

    nManager->deleteLater();

    return result;
}

QList<QPair<QString,QString>> CloudInterface_GoogleDrive::GetAllChildFolders(QString folderName)
{
    UpdateFoldersSnapshot();

    QList<QPair<QString,QString>> result;
    QString parentId = m_foldersSnapshot[folderName];

    if(parentId.isEmpty())
    {
        qDebug() << "GetAllChildFolders() error. Folder does not exist";
        //Todo: emit abort
        return result;
    }

    //Get metadata for every folder
    QNetworkAccessManager* nManager = new QNetworkAccessManager();
    QString baseUrl = "https://www.googleapis.com/drive/v3/files";
    for(auto e : m_foldersSnapshot.keys())
    {
        QString fileId = m_foldersSnapshot.value(e);
        QUrl url(baseUrl + "/" + fileId);

        QUrlQuery query;
        query.addQueryItem("fields", "id,name,parents");
        //query.addQueryItem("fields", "appProperties,capabilities,contentHints,createdTime,description,explicitlyTrashed,fileExtension,folderColorRgb,fullFileExtension,headRevisionId,iconLink,id,imageMediaMetadata,isAppAuthorized,kind,lastModifyingUser,md5Checksum,mimeType,modifiedByMeTime,modifiedTime,name,originalFilename,ownedByMe,owners,parents,permissions,properties,quotaBytesUsed,shared,sharedWithMeTime,sharingUser,size,spaces,starred,thumbnailLink,trashed,version,videoMediaMetadata,viewedByMe,viewedByMeTime,viewersCanCopyContent,webContentLink,webViewLink,writersCanShare");
        url.setQuery(query);

        QNetworkRequest request(url);
        request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());

        QEventLoop loop;
        QNetworkReply* reply = nManager->get(request);
        reply->setParent(nManager);
        connect(nManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
        loop.exec();


        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();

        QJsonArray parentsArr = root["parents"].toArray();
        if((parentsArr.size() > 0) && (parentsArr[0].toString() == parentId))
        {
            result.push_back(qMakePair(root.value("name").toString(), root.value("id").toString()));
        }
    }

    nManager->deleteLater();


    //qDebug() << result;
    return result;

}

QString CloudInterface_GoogleDrive::MakeOrGetShareLink(QString fileId)
{
    QString link;

    //Make folder shared
    QNetworkAccessManager* nManager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl(QString("https://www.googleapis.com/drive/v3/files/%1/permissions").arg(fileId)));
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");

    QJsonObject root;
    root.insert("role","reader");
    root.insert("type","anyone");
    root.insert("value","");
    QJsonDocument doc(root);

    QEventLoop loop;
    QNetworkReply* reply = nManager->post(request, doc.toJson());
    reply->setParent(nManager);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    //qDebug() << QJsonDocument::fromJson(reply->readAll());


    //Get the metadata with the link
    QUrl url(QString("https://www.googleapis.com/drive/v3/files/%1").arg(fileId));
    QUrlQuery query;
    query.addQueryItem("fields", "webViewLink");
    url.setQuery(query);
    request = QNetworkRequest(url);
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());

    QEventLoop loop2;
    reply = nManager->get(request);
    reply->setParent(nManager);
    connect(reply, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    loop2.exec();

    QJsonDocument answer = QJsonDocument::fromJson(reply->readAll());
    QJsonObject linkRoot = answer.object();

    link = linkRoot.value("webViewLink").toString();

    qDebug() << link;

    nManager->deleteLater();

    return link;

}

void CloudInterface_GoogleDrive::DeleteFolder(QString folder, bool isId)
{
    QString folderId;

    if(!isId)
    {
        folderId = m_foldersSnapshot[folder];
        if(folderId.isEmpty())
        {
            UpdateFoldersSnapshot();
            folderId = m_foldersSnapshot[folder];
            if(folderId.isEmpty())
                return; //TODO: emit error
        }
    }
    else
        folderId = folder;

    QNetworkAccessManager* nManager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl(QString("https://www.googleapis.com/drive/v3/files/%1").arg(folderId)));
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());

    QEventLoop loop;
    QNetworkReply* reply = nManager->deleteResource(request);
    reply->setParent(nManager);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    nManager->deleteLater();

}










