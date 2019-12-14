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

CloudInterface::CloudInterface(Authenticator* auth,QObject *parent): QObject(parent), m_authenticator(auth), m_isUploading(false)
{
    m_networkManager = new QNetworkAccessManager(this);

    //Connect all functions
    connect(m_authenticator, &Authenticator::OnSuccess, [&]()
    {
        //Close blocking window

    });


    //Wenn 503 zurück kommt ist upload unterbchen
    //1. Empty PUT request um den upload status abzufragen
    //2. Content-Range zum header hinzufügen: falls gesammtgröße bekannt: */göße annstonsten */*
    //3. Response verarbeiten: 200/201->OK 308->Incomplete 404->upload needs to be restarted
    //4. Im Response "Range" Header stehen wie viel bytes bereits empfangen wurden zb: bytes=0-42 <- ersten 43 bytes wurden empfangen
    //5. Restlichen bytes senden. Bei dem beispiel 43 bis filesize-1. Content-Range: bytes 43-filesize-1/filesize


    //simple upload uploadType=media -> 5mb or less. Check file size and choose upload type
    //simple upload + metatdata uploadType=multipart
    //Resumable upload uploadType=resumable <- beste choice für alles weil nur 1 zusätzlicher request und funzt auch mit kleinen dateien
    //Beispiel:
    //PUT https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable&upload_id=xa298sd_sdlkj2 HTTP/1.1
    //Content-Length: 1999957
    //Content-Range: bytes 43-1999999/2000000
    //[BYTES 43-1999999]


}



void CloudInterface::Authorize(){
    if(m_authenticator)
        m_authenticator->StartAuth();
}

void CloudInterface::TestUploadMultiPart()
{

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
    multiPart->setBoundary("foo_bar_baz");
    //Geh davon aus das Content_Lenght automatisch befüllt wird

    QFileInfo fileInfo("C:\\Users\\Fabi\\OneDrive\\TODO.txt");
    if(!fileInfo.exists())
    {
        qDebug() << "File not found";
        return;
    }


    QFile* File = new QFile("C:\\Users\\Fabi\\OneDrive\\TODO.txt", multiPart);//set multipart ao parent so it will be destroyed correctly
    if(!File->open(QIODevice::ReadOnly))
        return;


    QNetworkRequest request(QUrl("https://www.googleapis.com/upload/drive/v2/files?uploadType=multipart HTTP/1.1"));

    //HEAD
    QHttpPart initPart;
    initPart.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());
    //initPart.setRawHeader("Contet-Type", "multipart/related"); done in constructor

    //META
    QHttpPart metaPart;
    metaPart.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    QJsonObject metaObject;
    metaObject.insert("titel", fileInfo.fileName());
    QJsonDocument metaJson(metaObject);
    metaPart.setBody(metaJson.toJson());

    //FILE
    QHttpPart filePart;
    filePart.setRawHeader("Content-Type", GetContentTypeByExtension(fileInfo.suffix()).toUtf8());
    filePart.setBodyDevice(File);

    //////
    multiPart->append(initPart);
    multiPart->append(metaPart);
    multiPart->append(filePart);

    m_networkReply = m_networkManager->post(request, multiPart);
    multiPart->setParent(m_networkReply); //so it will be deleted with reply

    connect(m_networkManager, &QNetworkAccessManager::finished, [](){qDebug() << "NetworkAcessManager finished";});

    connect(m_networkReply, &QNetworkReply::uploadProgress,[=](qint64 sent, qint64 size){
        QString s;
        s = "" + QString::number(sent) + "/" + QString::number(size);
        qDebug() << s;
    });




}

void CloudInterface::TestUploadResumable()
{
    UploadFile("D:\\Downloads\\02. Alligatoah - Ein Problem Mit Alkohol.mp3");
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

void CloudInterface_GoogleDrive::UploadFiles(QStringList files)
{
    //Check if authorized.
    if(!m_authenticator->isGranted())
        return;

    foreach (auto path, files) {
        UploadFile(path);
    }
}

//TODO::private function, die eine liste aller cloud elemente abruft und im hauptfenster anzeit. <- soll am anfang aufgerufen werden und dann immer wenn änderungen passierten

void CloudInterface_GoogleDrive::UploadFile(QString filePath)
{
    QString path = filePath;
    if(!m_authenticator->isGranted())
        return;

    QFileInfo fileInfo(filePath);

    if(!fileInfo.exists())
        return;

    if(!m_networkManager)
        m_networkManager = new QNetworkAccessManager(this);

    m_currentFile = filePath;

    //Create MetaData
    QJsonObject root;
    root.insert("title", fileInfo.fileName());
    QJsonDocument body(root);
    QByteArray data = body.toJson();//TODO: Meta funktioniert nicht -> immer untiteld
    qDebug() << data.size() << QJsonDocument::fromJson(data).toJson() ;

    //Init Upload Session
    QNetworkRequest request;
    request.setUrl(QUrl("https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable"));
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());
    request.setRawHeader("Content-Lenght", QByteArray::number(data.size()));
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("X-Upload-Content-Type", GetContentTypeByExtension(fileInfo.suffix()).toUtf8());
    request.setRawHeader("X-Upload-Content-Length", QByteArray::number(fileInfo.size()));

    m_networkReply = m_networkManager->post(request, data);


    connect(m_networkManager, &QNetworkAccessManager::finished, this, &CloudInterface_GoogleDrive::HandleCreateUploadSessionReply);

}

//TODO: Test!!!! not testet jet
void CloudInterface_GoogleDrive::UploadFile(QString filePath, uint startBit)
{

    QString path = filePath;
    if(!m_authenticator->isGranted())
        return;

    QFileInfo fileInfo(filePath);

    if(!fileInfo.exists())
        return;

    if(!m_networkManager)
        m_networkManager = new QNetworkAccessManager(this);

    QNetworkRequest request(m_currentUploadUrl);
    request.setRawHeader("Content-Lenght", QByteArray::number(fileInfo.size()-startBit));
    request.setRawHeader("Content-Range", QString("bytes " + QString::number(startBit) +"-"+ QString::number(fileInfo.size()-1)+"/*").toUtf8());
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    file.seek(startBit);
    QByteArray restOfFile = file.read((fileInfo.size()-startBit));
    file.close();

    QNetworkReply* reply = m_networkManager->put(request, restOfFile);
    m_networkManager->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, &CloudInterface_GoogleDrive::HandleUploadingReply);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);

}

void CloudInterface_GoogleDrive::HandleCreateUploadSessionReply(QNetworkReply *reply)
{
    if(!m_networkManager)
        m_networkManager = new QNetworkAccessManager();

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        //Handle Error
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = document.object();
        QJsonObject error = root["error"].toObject();
        QJsonValue code = error.value("code");
        QJsonValue message = error.value("message");
        qDebug() << "code: " + QString::number(code.toInt()) << "  message: " << message.toString();

        return;
    }


    //qDebug() << reply->rawHeaderPairs();
    QString Content_Lenght = QString::fromUtf8(reply->rawHeader("Content-Lenght"));
    QString Content_Type = QString::fromUtf8(reply->rawHeader("Content-Type"));
    QString X_GUploader_UploadID = QString::fromUtf8(reply->rawHeader("X-GUploader-UploadID"));
    QString Location = QString::fromUtf8(reply->rawHeader("Location"));
    QString Cache_Control = QString::fromUtf8(reply->rawHeader("Cache-Control"));
    QString Date = QString::fromUtf8(reply->rawHeader("Date"));
    m_currentUploadUrl = Location;


    disconnect(m_networkManager, &QNetworkAccessManager::finished, this, &CloudInterface_GoogleDrive::HandleCreateUploadSessionReply);
    m_networkReply->deleteLater();

    //Start upload
    QNetworkRequest request(Location);
    QFile file(m_currentFile);
    QFileInfo fileInfo(m_currentFile);

    if(!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data(file.readAll()); //TODO: Without loading it into memory
    file.close();

    //TODO: Check return code for 200.

    request.setRawHeader("Content-Lenght", QByteArray::number(fileInfo.size()));
    request.setRawHeader("Content-Type", GetContentTypeByExtension(fileInfo.suffix()).toUtf8());
    m_networkManager->put(request, data);

    connect(m_networkManager, &QNetworkAccessManager::finished, this, &CloudInterface_GoogleDrive::HandleUploadingReply);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &QNetworkAccessManager::deleteLater);


}

void CloudInterface_GoogleDrive::HandleUploadingReply(QNetworkReply *reply)
{
    if(!m_networkManager)
        m_networkManager = new QNetworkAccessManager();

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status != 200 && status != 201)
    {
        if(status == 308)
        {
            //Request upload status
            QNetworkAccessManager* nManager = new QNetworkAccessManager();
            QNetworkRequest request(m_currentUploadUrl);
            request.setRawHeader("Content-Length", "0");
            request.setRawHeader("Content-Range", "bytes */*");
            QNetworkReply* reply = nManager->put(request, QByteArray());
            nManager->setParent(reply);
            connect(reply, &QNetworkReply::finished, [&](){
                if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 308)
                {
                    //Continue Uploading file
                    QString range = QString::fromUtf8(reply->rawHeader("Range"));
                    auto s1 = range.split("=");
                    auto receivedBytes = s1[1].split("-");
                    UploadFile(m_currentFile, receivedBytes[1].toUInt()+1);

                }

            });
            connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);


        }
        else
        {
            //restart upload
            UploadFile(m_currentFile);
        }

    }

    m_currentFile = "";
    qDebug() << "Status: " + QString::number(status);
    qDebug() << QJsonDocument::fromJson(reply->readAll());
}



