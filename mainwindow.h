#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

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
    void on_actionOpen_triggered();

private:
    void update_text_areas(QString &text);
    Ui::MainWindow *ui;
    QString currentFile = "";

    QString m_hexdump_savedText;
    QString m_offset_savedText;
    QString m_ascii_data_savedText;

};
#endif // MAINWINDOW_H
