#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QDragEnterEvent;
class QDropEvent;
class CloudInterface;
class CloudInterface_GoogleDrive;
class QTableWidget;
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
private slots:

private:
    void parseJsonSecret(GoogleConfig& gConfig);
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private:
    Ui::MainWindow *ui;
    QTableWidget* m_tableWidget;
    CloudInterface* m_cloudInterface;
    QString m_AppFolderId;
    QString m_AppName;

};
#endif // MAINWINDOW_H
