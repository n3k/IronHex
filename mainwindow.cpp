#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->gridLayoutWidget);

    //this->m_offset_savedText = ui->textEdit_offset->toPlainText();
    //this->m_ = m_ui->myLabel->text();
    //m_ui->myLabel->setText(this->m_savedText.arg("Default text"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_text_areas(QString &text) {

    // Prepare offset text area
    QString offset_text = "";
    for (int i = 0; i < text.length(); i += 16) {
        QString str;
        offset_text += str.sprintf("%08xh\n", i);
    }

    // Prepare ascii-data text area
    QString ascii_data_text = "";
    size_t text_len = text.length();
    size_t p = 0;
    while (p < text_len) {
        size_t rem = text_len - p;
        if (rem < 16) {
            ascii_data_text += text.mid(p, p+rem);
        } else {
            ascii_data_text += text.mid(p, p+16);
        }
        p += 16;
        ascii_data_text += "\n";
    }

    // Prepare hexdump text area
    QString hexdump_text = "";
//    for (int i = 0; i < text.length(); i++) {
//        QString str;
//        if ( (i + 1) % 16 != 0 ) {
//            hexdump_text += str.sprintf("%02X  ", text.at(i));
//        } else {
//            hexdump_text += str.sprintf("%02X\n", text.at(i));
//        }
//    }


    // Set text areas
    ui->textEdit_asciidata->setPlainText(ascii_data_text);
    ui->textEdit_offset->setPlainText(offset_text);
    ui->textEdit_hexdump->setPlainText(hexdump_text);

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
    QTextStream in(&file);
    QString text = in.readAll();
    update_text_areas(text);
    file.close();
}

