#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "authenticator.h"
#include "cloudinterface.h"
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDebug>
#include <windows.h>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <uploaddialog.h>


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{

    m_AppName = "ImmoCloud";

    ui->setupUi(this);
    setAcceptDrops(true);
    m_listWidget = ui->listWidget;
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    GoogleConfig googleConfig;
    parseJsonSecret(googleConfig);

    m_cloudInterface = new CloudInterface_GoogleDrive(googleConfig.clientID, googleConfig.clientSecret, this);
    connect(m_cloudInterface, &CloudInterface_GoogleDrive::IsReady, [&](){
        //When authorized -> create root folder

        QProgressDialog prog("Getting folder infos", nullptr, 0, 2, this); //Just for the user to see is smthing happening
        prog.show();
        prog.setWindowModality(Qt::WindowModal);
        prog.setValue(0);
        m_AppFolderId = m_cloudInterface->CreateFolder(m_AppName, "root");
        prog.setValue(1);
        m_cloudInterface->MakeOrGetShareLink(m_AppFolderId);
        prog.setValue(2);
        UpdateFolderList();
        prog.close();
    });

    m_cloudInterface->Authorize();

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::ListItemClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateFolderList()
{
    QProgressDialog prog("Refreshing Folder List", nullptr, 0, 2, nullptr);
    prog.setValue(1);
    prog.setModal(true);
    prog.show();

    //Get all folders in app-folder on drive
    QList<QPair<QString,QString>> folders = m_cloudInterface->GetAllChildFolders(m_AppName);
    m_listWidget->clear();
    for(auto& folder : folders)
    {
        QListWidgetItem* item = new QListWidgetItem(folder.first);
        item->setStatusTip(folder.second);
        m_listWidget->addItem(item);
    }

    prog.setValue(2);
}

void MainWindow::ListItemClicked(QListWidgetItem *item)
{
    if(!item)
        return;

    QString folderName = item->text();
    QString folderId = item->statusTip();

    //Open Dialog with link
    QString shareLink = m_cloudInterface->MakeOrGetShareLink(folderId);


    QMessageBox box(QMessageBox::NoIcon, "Share Link", shareLink, QMessageBox::Ok, this);
    box.setTextInteractionFlags(Qt::TextSelectableByMouse);
    box.exec();
}

void MainWindow::OpenUploadDialog(QStringList filePaths)
{

    UploadDialog* uDialog = new UploadDialog(m_cloudInterface, m_AppFolderId, this);
    uDialog->setWindowTitle("Upload Files");
    uDialog->AddToList(filePaths);
    uDialog->exec();
    UpdateFolderList();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasUrls())
        e->acceptProposedAction();

}

void MainWindow::dropEvent(QDropEvent *e)
{
    QStringList pathList;

    foreach (auto url, e->mimeData()->urls()) {
        pathList.append(url.toLocalFile());

    }

    e->acceptProposedAction();

    OpenUploadDialog(pathList);
}

void MainWindow::parseJsonSecret(GoogleConfig& gConfig)
{
    //Make sure everything needed to authenticate is available
    QString path = QDir::currentPath() + "/config/google_drive";
    QDir dir;

    if(!dir.exists(path))
       dir.mkpath(path);


    QFileInfo info(path+"/client_secret.json");
    if(!info.exists())
    {
        MessageBoxA(nullptr, "The Client Secret Json is missing in the config folder", "Client Secret Missing", MB_OK);
        exit(0);
    }

    //Load everything needed from the json file (Downlaodable from google)
    QFile secretFile(path +"/client_secret.json");
    if(!secretFile.open(QIODevice::ReadOnly))
        return;



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

void MainWindow::on_UploadButton_clicked()
{
    OpenUploadDialog(QStringList());
}

void MainWindow::on_actionQuit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::DeleteItem()
{

    for(int i = 0; i < m_listWidget->selectedItems().size(); ++i)
    {
        QListWidgetItem* item = m_listWidget->takeItem(m_listWidget->currentRow());
        m_cloudInterface->DeleteFolder(item->statusTip(), true);
        delete item;
    }

    UpdateFolderList();
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    QPoint globalPos = m_listWidget->mapToGlobal(pos);

    QMenu menu;
    menu.addAction("Delete", this, &MainWindow::DeleteItem);
    menu.exec(globalPos);
}

void MainWindow::on_RefreshButton_clicked()
{
    UpdateFolderList();
}
