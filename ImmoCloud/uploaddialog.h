#ifndef UPLOADDIALOG_H
#define UPLOADDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QListWidget;
class CloudInterface;
class QTextEdit;

namespace Ui {
class UploadDialog;
}

class UploadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UploadDialog(CloudInterface* cloudInterface , QString AppFolderId ,QWidget *parent = nullptr);
    ~UploadDialog();
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

public slots:
    void AddToList(QString);
    void AddToList(QStringList);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::UploadDialog *ui;
    QDialogButtonBox* m_buttonBox;
    QListWidget* m_listWidgetFiles;
    QTextEdit* m_folderName;
    CloudInterface* m_interface;
    QString m_rootFolderId;
};

#endif // UPLOADDIALOG_H
