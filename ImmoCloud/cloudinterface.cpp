#include "cloudinterface.h"
#include "authenticator.h"

#include "windows.h"
#include <qnetworkreply.h>

#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <QFile>

CloudInterface::CloudInterface(Authenticator* auth,QObject *parent): QObject(parent), m_authenticator(auth)
{
    m_networkManager = new QNetworkAccessManager(this);

    //Connect all functions
    connect(m_authenticator, &Authenticator::OnSuccess, [&]()
    {
        //Close blocking window





    });

    //1. & 2. Create a request POSt like below
    //media post->/upload/drive/v2/files
    //metadata post -> /drive/v2/files
    //https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable
    //PUT https://www.googleapis.com/upload/drive/v2/files/[FILE_ID]?uploadType=resumable
    //3.Add metadata to the request body in json format otherwhise leave empty
    //4.Add HTTP headers: optional MIME type.(X-Upload-Content-Type) default: application/octet-stream
    //X-Upload-Content-Lenght<-optional: number of bytes sent in each part
    //Content-Type<-Required if you have metadata: application/json; charset=UTF-8
    //Content-Lenght<-Required unless you use chunked transfer encoding. Set to the number of bytes in the body of this initial request.
    //Beispiel:

    //POST https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable HTTP/1.1
    //Authorization: Bearer [YOUR_AUTH_TOKEN]
    //Content-Length: 38
    //Content-Type: application/json; charset=UTF-8
    //X-Upload-Content-Type: image/jpeg
    //X-Upload-Content-Length: 2000000
    //{
      //"title": "myObject"
    //}
    //5. Es kommt ein Http satus code zurück(200 falls ok) der eine URI enthält die für upload genutzt werden kann

    //FILE UPLOADEN:
    //1.Create PUT request to the received url
    //2.Add the files data to the request body
    //3.Add Conten-Lenght HTTP header <- numb of bytes in file
    //4.Send request <-if it was successfull return code is 200 OK or 201 Created

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
    initPart.setRawHeader("Authorization", ("Bearer " + m_authenticator->m_authFlow->token()).toUtf8());
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
    UploadFile("C:\\Users\\Fabi\\OneDrive\\TODO.txt");
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
    root.insert("titel", fileInfo.fileName());
    QJsonDocument body(root);
    QByteArray data = body.toJson();//TODO: Meta funktioniert nicht -> immer untiteld

    //Init Upload Session
    QNetworkRequest request;
    request.setUrl(QUrl("https://www.googleapis.com/upload/drive/v2/files?uploadType=resumable"));
    request.setRawHeader("Authorization", ("Bearer " + m_authenticator->GetToken()).toUtf8());
    request.setRawHeader("Content-Lenght", QByteArray::number(data.size()));
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("X-Upload-Content-Type", GetContentTypeByExtension(fileInfo.suffix()).toUtf8());

    m_networkReply = m_networkManager->post(request, data);


    connect(m_networkManager, &QNetworkAccessManager::finished, this, &CloudInterface_GoogleDrive::HandleCreateUploadSessionReply);

}

void CloudInterface_GoogleDrive::HandleCreateUploadSessionReply(QNetworkReply *reply)
{
    //Parse response:
    //If data is 0. Session was sucessfull created
    QString Content_Lenght = QString::fromUtf8(reply->rawHeader("Content-Lenght"));
    if(Content_Lenght > 0)
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
    QString Content_Type = QString::fromUtf8(reply->rawHeader("Content-Type"));
    QString X_GUploader_UploadID = QString::fromUtf8(reply->rawHeader("X-GUploader-UploadID"));
    QString Location = QString::fromUtf8(reply->rawHeader("Location"));
    QString Cache_Control = QString::fromUtf8(reply->rawHeader("Cache-Control"));
    QString Date = QString::fromUtf8(reply->rawHeader("Date"));

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

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status != 200 && status != 201)
    {
        //TODO: //if not resume or start again. Lieber switch case machen
    }

    qDebug() << "Status: " + QString::number(status);
    qDebug() << QJsonDocument::fromJson(reply->readAll());
}



