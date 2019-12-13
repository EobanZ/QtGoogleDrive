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

    const QString ClientIdentifier("57346428628-1s9lp9rar6qf4m7gk46lkg7k16ju6ogj.apps.googleusercontent.com");
    const QString ClientSecret("WF4wzPlnti-dnZTn63uvZ6Fn");


    m_cloudInterface = new CloudInterface_GoogleDrive(ClientIdentifier, ClientSecret, this);

    m_cloudInterface->Authorize();
    //Open window that can only be closed if Authentifiern emits OnSuccess() -> should close the window. BLOCK HERE with window.exec()??


    //Slot erstellen für UploadButtonPressed()
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
   m_cloudInterface->TestUploadResumable();
}
