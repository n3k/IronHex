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

struct TableContext {
    QTableWidget *table;
    QByteArray _content;
    /// The number of required rows for the current content
    size_t _content_rows;
    /// The col number on which the content ends in the last row
    size_t _content_last_column;
    QMap<int, int> string_map;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void re_load_ascii_area(struct TableContext *t_ctx);
    void do_diff();

private slots:
    void on_actionOpen_triggered();

    void on_tableWidget_ctrlCPressed(QModelIndexList selected);
    void on_tableWidget_2_ctrlCPressed(QModelIndexList selected);
    void on_tableWidget_ctrlVPressed(int r, int c);
    void on_tableWidget_2_ctrlVPressed(int r, int c);


    void on_actionSave_As_triggered();
    void on_actionSave_triggered();

    void on_actionClose_triggered();
    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionExit_triggered();

    void on_actionLoad2_triggered();

    void on_pushButton_goto_offset_clicked();

private:
    bool bInitialLoadDone;
    void re_load_table_content(QByteArray &text, struct TableContext *t_ctx);
    void reload_content(struct TableContext *t_ctx);
    void next_hex_cell(int *r, int *c, struct TableContext *t_ctx);
    void find_strings_in_data(QByteArray &data, struct TableContext *t_ctx);
    void set_strings_bg_color(struct TableContext *t_ctx);
    void map_offset_to_cell(size_t offset, int *r, int *c);

    void handle_ctrl_c(QModelIndexList selected);
    void handle_ctrl_v(int r, int c, struct TableContext *t_ctx);



    Ui::MainWindow *ui;

    QString currentFile1 = "";
    struct TableContext table1_ctx;

    QRegularExpression rx_copypaste;


    QString currentFile2 = "";
    struct TableContext table2_ctx;




};
#endif // MAINWINDOW_H
