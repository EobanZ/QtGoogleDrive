#include "uploaddialog.h"
#include "ui_uploaddialog.h"
#include "QDialogButtonBox">
#include "cloudinterface.h"
#include <QMimeData>
#include <QDragEnterEvent>

UploadDialog::UploadDialog(CloudInterface* interface, QString AppFolderId, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadDialog)
{
    ui->setupUi(this);

    m_listWidgetFiles = ui->listWidgetFiles;
    m_buttonBox = ui->buttonBox;
    m_folderName = ui->textEditFolder;
    m_folderName->setFocus();

    setAcceptDrops(true);

    m_interface = interface;
    m_rootFolderId = AppFolderId;

}

UploadDialog::~UploadDialog()
{
    delete ui;
}

void UploadDialog::dragEnterEvent(QDragEnterEvent *e)
{

    if(e->mimeData()->hasUrls())
        e->acceptProposedAction();

}

void UploadDialog::dropEvent(QDropEvent *e)
{
    QStringList pathList;

    foreach (auto url, e->mimeData()->urls()) {
        pathList.append(url.toLocalFile());

    }

    e->acceptProposedAction();

    AddToList(pathList);

}

void UploadDialog::AddToList(QString s)
{
    if(s.isEmpty())
        return;

    m_listWidgetFiles->addItem(new QListWidgetItem(s));
}

void UploadDialog::AddToList(QStringList sl)
{
    if(sl.isEmpty())
        return;

    for(auto& e : sl)
    {
        AddToList(e);
    }
}


void UploadDialog::on_buttonBox_accepted()
{
    QString newFolderName = m_folderName->toPlainText();

    if(m_listWidgetFiles->count() <= 0)
        return;

    if(newFolderName.isEmpty())
    {
        //TODO: disable ok button untill folder got an name
        return;
    }




    QStringList allPaths;
    for(int i(0); i < m_listWidgetFiles->count(); i++)
    {
        allPaths.append(m_listWidgetFiles->item(i)->text());
    }


    QString folderID = m_interface->CreateFolder(newFolderName, m_rootFolderId);
    m_interface->UploadFiles(allPaths, folderID);


}
