#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QModelIndexList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define COLUMN_OFFSET 0
#define COLUMN_ASCII_DATA 17

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_tableWidget_ctrlCPressed(QModelIndexList selected);
    void on_tableWidget_ctrlVPressed(int r, int c);

private:
    void update_text_areas(QByteArray &text);
    Ui::MainWindow *ui;
    QString currentFile = "";

    QString m_hexdump_savedText;
    QString m_offset_savedText;
    QString m_ascii_data_savedText;



};
#endif // MAINWINDOW_H
