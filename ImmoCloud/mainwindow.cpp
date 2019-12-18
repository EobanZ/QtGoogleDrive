#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "authenticator.h"
#include "cloudinterface.h"
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDebug>
#include <windows.h>



MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);


    m_AppName = "ImmoCloud";

    GoogleConfig googleConfig;
    parseJsonSecret(googleConfig);

    m_cloudInterface = new CloudInterface_GoogleDrive(googleConfig.clientID, googleConfig.clientSecret, this);

    m_cloudInterface->Authorize();
    //Open window that can only be closed if Authentifiern emits OnSuccess() -> should close the window. BLOCK HERE with window.exec()??

    //TODO: Creat or get AppFolder after Interface is Authorized
    //TODO: create slot(createAppFolder) and connect with CloudInterface Success()
    QTimer::singleShot(1000, [&](){m_AppFolderId = m_cloudInterface->CreateFolder(m_AppName, "root");});

    //QStringList list; //<--created from drop event. Maybe make global variable to clear and fill when drop event occours. Better to make new window with title, and append to the lsit. when ok button pressed the list filse gets uploaded
    //m_cloudInterface->UploadFiles(list);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasUrls())
        e->acceptProposedAction();

}

void MainWindow::dropEvent(QDropEvent *e)
{
    QStringList pathList;

    //Open Upload Window

    foreach (auto url, e->mimeData()->urls()) {
        pathList.append(url.toLocalFile());
        //funzt
        //MessageBoxA(nullptr,url.toLocalFile().toLocal8Bit().data(), "dropped", MB_OK);
        //Open new Window with list of all files. Oder upload buttonen und dann fenster <- Dialog : "Upload, Cancel"
    }

    e->acceptProposedAction();
}

void MainWindow::on_pushButton_clicked()
{
    QStringList files;
    files.push_back("C:\\Users\\Fabi\\Pictures\\Wallpaper\\chl1lq6.jpg");
    files.push_back("C:\\Users\\Fabi\\Pictures\\Uplay\\Tom Clancy's Rainbow Six® Siege\\Tom Clancy's Rainbow Six® Siege2017-11-18-0-19-6.jpg");
    m_cloudInterface->UploadFiles(files, m_AppFolderId);

    //m_cloudInterface->DeleteFolderWithFiles(QString());
}

void MainWindow::parseJsonSecret(GoogleConfig& gConfig)
{
    //Change this for different Cloud service. Better to do this in the constructor of eacht authenticator class, but i dont want to rewrite the who thing
    //Make sure everything needed to authenticate is available
    QString path = QDir::currentPath() + "/config/google_drive/";
    QDir dir;

    if(!dir.exists(path))
       dir.mkpath(path);

    //Load everything needed from the json file (Downlaodable from google)
    QFile secretFile(path +"/client_secret.json");
    if(!secretFile.open(QIODevice::ReadOnly))
        return;

    QFileInfo info(path+"/client_secret.json");
    if(!info.exists())
    {
        MessageBoxA(nullptr, "The Client Secret Json is missing in the config folder", "Client Secret Missing", MB_OK);
        exit(0);
    }

    QJsonDocument clienSecretDoc = QJsonDocument::fromJson(secretFile.readAll());
    secretFile.close();

    QJsonObject root = clienSecretDoc.object();
    QJsonObject web = root["web"].toObject();
    QJsonValue clientID = web.value("client_id");
    QJsonValue projectID = web.value("project_id");
    QJsonValue authUri = web.value("auth_uri");
    QJsonValue tokenUri = web.value("token_uri");
    QJsonValue authProvider = web.value("auth_provider_x509_cer_url");
    QJsonValue clientSecret = web.value("client_secret");
    QJsonArray redirectUris = web["redirect_uris"].toArray();
    QJsonValue redirectUri = redirectUris[0];

    gConfig.clientSecret = clientSecret.toString();
    gConfig.clientID = clientID.toString();
    gConfig.authUri = authUri.toString();
    gConfig.projectID = projectID.toString();
    gConfig.authProvider = authProvider.toString();
    gConfig.tokenUri = tokenUri.toString();
    gConfig.redirectUri = redirectUri.toString();

}
