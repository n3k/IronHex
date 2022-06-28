#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "itemdelegate.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->gridLayoutWidget);

    this->ui->tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    ItemDelegate *itDelegate = new  ItemDelegate;
    ui->tableWidget->setItemDelegate(itDelegate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString get_non_ascii_chars_version(QString &text) {
    QString result;
    result.reserve(text.size());
    foreach (const QChar& c, text) {
        if (c.isPrint()) {
            result += c;
        } else {
            result += ".";
        }
    }

    return result;
}

void MainWindow::update_text_areas(QByteArray &text) {

    size_t text_len = text.length();
    size_t lines_16 = text_len >> 4;
    lines_16 += ((text_len & 0b111) != 0) ? 1: 0;
    size_t rows = lines_16;

    const size_t column_num = ui->tableWidget->columnCount();

    QVector<QString> offset_vec;
    QVector<QByteArray> data_vec;

    size_t p = 0;
    QString str1;
    QByteArray str;
    while (p < text_len) {
        size_t rem = text_len - p;
        offset_vec.push_back(str1.sprintf("%08zxh", p));
        if (rem < 16) {
            str = text.mid(p, rem);
        } else {
            str = text.mid(p, 16);
        }

        data_vec.push_back(str);
        p += 16;
    }

    for(size_t i = 0; i < rows; i++) {
        ui->tableWidget->insertRow ( ui->tableWidget->rowCount() );

        QByteArray data_line = data_vec.at(i);

        for(size_t j = 0; j < column_num; j++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(i, j);
            if(!item) {
                item = new QTableWidgetItem();
                ui->tableWidget->setItem(i, j, item);
            }

            if (j == COLUMN_OFFSET) {
                item->setText(offset_vec.at(i));
                item->setBackgroundColor(Qt::lightGray);
                item->setFlags(Qt::ItemIsEnabled);
            } else if (j == COLUMN_ASCII_DATA) {
                QString ascii_data = QString(data_line);
                item->setText(get_non_ascii_chars_version(ascii_data));
                item->setBackgroundColor(Qt::lightGray);
                item->setFlags(Qt::ItemIsEnabled);
            } else {
                // HEXDUMP COLUMNS
                QString hexbyte_str;
                int column_idx = j-1;
                if (column_idx < data_line.size()) {
                    unsigned char b = (unsigned char) data_line.at(column_idx);
                    item->setText(hexbyte_str.sprintf("%02X", b));
                    item->setTextAlignment(Qt::AlignCenter);
                }
            }
        }
    }

    ui->tableWidget->resizeColumnsToContents();

    offset_vec.clear();
    data_vec.clear();
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open the file");
    currentFile = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
    }
    setWindowTitle(filename);
    //QTextStream in(&file);
    QByteArray text = file.readAll();
    update_text_areas(text);
    file.close();
}

/*
Feature list
. Determine strings and mark them with a different background colour in the hexdump
. Implement copy/paste
. Implement copy from selection
*/
