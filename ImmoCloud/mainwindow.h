#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QDragEnterEvent;
class QDropEvent;
class CloudInterface;

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
    void on_pushButton_clicked();

private:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private:
    Ui::MainWindow *ui;
    CloudInterface* m_cloudInterface;

};
#endif // MAINWINDOW_H
