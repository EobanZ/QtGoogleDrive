#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QDragEnterEvent;
class QDropEvent;
class CloudInterface;
class CloudInterface_GoogleDrive;
class QListWidget;
class QListWidgetItem;
struct GoogleConfig;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void UpdateFolderList();
    void ListItemClicked(QListWidgetItem* item);
    void OpenUploadDialog(QStringList filePaths);

private slots:
    void on_UploadButton_clicked();
    void on_actionQuit_triggered();
    void DeleteItem();
    void showContextMenu(const QPoint&);

    void on_RefreshButton_clicked();

private:
    void parseJsonSecret(GoogleConfig& gConfig);
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private:
    Ui::MainWindow *ui;
    QListWidget* m_listWidget;
    CloudInterface* m_cloudInterface;
    QString m_AppFolderId;
    QString m_AppName;

};
#endif // MAINWINDOW_H
