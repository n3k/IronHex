#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QModelIndexList>
#include <QRegularExpression>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define COLUMN_OFFSET 0
#define COLUMN_FIRST_HEX_BYTE 1
#define COLUMN_LAST_HEX_BYTE  16
#define COLUMN_ASCII_DATA 17

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void re_load_ascii_area();

private slots:
    void on_actionOpen_triggered();

    void on_tableWidget_ctrlCPressed(QModelIndexList selected);
    void on_tableWidget_ctrlVPressed(int r, int c);


    void on_actionSave_As_triggered();
    void on_actionSave_triggered();

    void on_actionClose_triggered();
    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionExit_triggered();

private:
    bool bInitialLoadDone;
    void re_load_table_content(QByteArray &text);
    void reload_content();
    void next_hex_cell(int *r, int *c);
    void find_strings_in_data(QByteArray &data);
    void set_strings_bg_color();
    void map_offset_to_cell(size_t offset, int *r, int *c);


    Ui::MainWindow *ui;
    QString currentFile = "";

    QByteArray _content;
    /// The number of required rows for the current content
    size_t _content_rows;
    /// The col number on which the content ends in the last row
    size_t _content_last_column;

    QRegularExpression rx_copypaste;

    QMap<int, int> qstring_map;



};
#endif // MAINWINDOW_H
