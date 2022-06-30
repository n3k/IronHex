#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "itemdelegate.h"
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    bInitialLoadDone = false;

    this->setCentralWidget(ui->gridLayoutWidget);

    this->rx_copypaste = QRegularExpression("([aA|bB|cC|dD|eE|fF|\\d]{2}\\s)*[aA|bB|cC|dD|eE|fF|\\d]{2}");

    this->ui->tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    ItemDelegate *itDelegate = new  ItemDelegate;
    itDelegate->set_main_window(this);
    ui->tableWidget->setItemDelegate(itDelegate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString get_non_ascii_chars_version(QByteArray &ba) {
    QString result;
    result.reserve(ba.size());
    foreach (const char& c, ba) {
        if (c < 0x20 || c > 0x7f) {
            result += ".";
        } else {
            result += QChar::fromLatin1(c);
        }
    }

    return result;
}

void MainWindow::reload_content() {
    _content.clear();

    QString text = "";
    for(int i = 0 ; i < this->_content_rows; i++) {
        for (int j = COLUMN_FIRST_HEX_BYTE; j <= COLUMN_LAST_HEX_BYTE; j++) {

            QTableWidgetItem *item = ui->tableWidget->item(i, j);

            if (i == (_content_rows - 1) && j > _content_last_column) {
                // This case is where we're on the last row and we consumed
                // all the columns with data for this row.
                break;
            }

//            if (i >= (_content_rows - 1))
//                printf("Realoding cell r[%d]c[%d]\n", i, j);

            std::string hexbyte_str = item->text().toStdString();

            unsigned long v = strtoul(hexbyte_str.c_str(), 0, 16);
//            if (hexbyte_str == "0D") {
//                QString str;
//                QMessageBox::warning(this, "Warning", str.sprintf("0D found: %02x", v));
//            }
            _content.push_back(v & 0xff);
        }
    }
}

/// This method only reloads the ascii-strings portion of the table
void MainWindow::re_load_ascii_area() {
    // Get the bytes from the current hexdump and reload
    reload_content();

    size_t text_len = _content.length();
    QVector<QByteArray> data_vec;

    size_t p = 0;
    QByteArray str;
    while (p < text_len) {
        size_t rem = text_len - p;
        if (rem < 16) {
            str = _content.mid(p, rem);
        } else {
            str = _content.mid(p, 16);
        }

        data_vec.push_back(str);
        p += 16;
    }

    for(int i = 0 ; i < _content_rows; i++) {
        QByteArray data_line = data_vec.at(i);

        QTableWidgetItem *item = ui->tableWidget->item(i, COLUMN_ASCII_DATA);
        //printf("Reloading ascii row: %d - %p\n", i, item);
        item->setText(get_non_ascii_chars_version(data_line));
        item->setBackgroundColor(Qt::lightGray);
        item->setFlags(Qt::ItemIsEnabled);
    }
}


/// This method will load or reload the entire content of the table
/// based on the input bytearray
void MainWindow::re_load_table_content(QByteArray &text) {

    ui->tableWidget->clearContents();

    size_t text_len = text.size();
    size_t lines_16 = text_len >> 4;
    lines_16 += ((text_len & 0b1111) != 0) ? 1: 0;
    size_t needed_rows = lines_16;
    this->_content_rows = needed_rows;

    const size_t column_num = ui->tableWidget->columnCount();

    QVector<QString> offset_vec;
    QVector<QByteArray> data_vec;

    size_t p = 0;
    QString str1;
    QByteArray ba;
    while (p < text_len) {
        size_t rem = text_len - p;
        offset_vec.push_back(str1.sprintf("%08zxh", p));
        if (rem < 16) {
            ba = text.mid(p, rem);
        } else {
            ba = text.mid(p, 16);
        }

        data_vec.push_back(ba);
        p += 16;
    }

    // Allocate ROWs for current content
    int additional_rows = needed_rows - ui->tableWidget->rowCount();

    while (additional_rows > 0) {
        ui->tableWidget->insertRow ( ui->tableWidget->rowCount() );
        additional_rows--;
    }

    size_t offset = 0;
    for(size_t i = 0; i < needed_rows; i++) {

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
                item->setText(get_non_ascii_chars_version(data_line));
                item->setBackgroundColor(Qt::lightGray);
                item->setFlags(Qt::ItemIsEnabled);

            } else {
                // HEXDUMP COLUMNS
                if (offset >= text_len) {
                    continue; // Continue because we need to still set COLUMN_ASCII_DATA for this row
                }
                unsigned char b = text.at(offset);

                QString hexbyte_str;                
                item->setText(hexbyte_str.sprintf("%02X", b));
                item->setTextAlignment(Qt::AlignCenter);
                offset++;

                if (offset == text_len) {
                    _content_last_column = j;
                }
            }
        }
    }

    ui->tableWidget->resizeColumnsToContents();

    offset_vec.clear();
    data_vec.clear();

    bInitialLoadDone = true;
}

size_t ascii_strlen(char *s) {
    size_t size = 0;
    while (*s >= 0x20 && *s <=0x7f) {
        size++;
        s++;
    }
    return size;
}

void MainWindow::map_offset_to_cell(size_t offset, int *r, int *c) {
    *r = offset >> 4;
    *c = (offset & 0b1111) + COLUMN_FIRST_HEX_BYTE;
}

void MainWindow::find_strings_in_data(QByteArray &data) {
    qstring_map.clear();

    int i = 0;
    size_t str_len = 0;
    char *ptr;
    while (i < data.length()) {
        unsigned char c = (unsigned char ) data.at(i);
        if (c >= 0x20 && c <= 0x7f) {
            ptr = data.data();
            ptr += i;
            str_len = ascii_strlen(ptr);
            if (str_len >= 3) {
                //QString str;
                //QMessageBox::warning(this, "Warning", str.sprintf("%d:%d:%s", i, str_len, ptr));
                this->qstring_map[i] = str_len;
                i += str_len;
            } else {
                i += 2;
            }
        } else {
            i += 1;
        }
    }
}

void MainWindow::on_actionOpen_triggered()
{
    _content.clear();
    this->ui->tableWidget->setRowCount(0);
    bInitialLoadDone = false;

    QString filename = QFileDialog::getOpenFileName(this, "Open the file");
    currentFile = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
    }

    setWindowTitle(filename);

    QByteArray text = file.readAll();

    // find strings
    find_strings_in_data(text);

    // fill table
    //this->ui->tableWidget->clear();
    re_load_table_content(text);

    // set color for strings
    set_strings_bg_color();


    file.close();
}

void MainWindow::set_strings_bg_color() {

    for (auto it = qstring_map.begin(); it != qstring_map.end(); ++it) {
        int cur_row = 0;
        int cur_col = 0;
        map_offset_to_cell(it.key(), &cur_row, &cur_col);

        for (int i = 0; i < it.value(); i++) {
            //QString str;
            //QMessageBox::warning(this, "Warning", str.sprintf("r[%d]c[%d]", cur_row, cur_col));
            QTableWidgetItem *item = this->ui->tableWidget->item(cur_row, cur_col);
            item->setBackgroundColor(Qt::green);
            next_hex_cell(&cur_row, &cur_col);
        }
    }
}

/*
Feature list
. Determine strings and mark them with a different background colour in the hexdump
. Implement Insert Zeros
. implement save
*/

/// This function receives a row and column index and returns the
/// next row and column indexes
void MainWindow::next_hex_cell(int *r, int *c) {

    QTableWidgetItem *item;
    bool bNextRow = false;

    int col = *c;
    if (col < COLUMN_LAST_HEX_BYTE) {
        col++;
    } else {
        col = COLUMN_FIRST_HEX_BYTE;
        bNextRow = true;
    }
    // update c
    *c = col;

    int row = *r;
    if (row == (_content_rows - 1)) {

        // If we re on the last ROW and our increased COLUMN is greater than what we tracked before
        // Then increase the _content_last_column (to col)
        if (col > _content_last_column) {
             _content_last_column = col;
             // Add new item
             item = this->ui->tableWidget->item(row, col);
             if (item == NULL) {
                 QMessageBox::warning(this, "Warning", "adding new COL");
                 ui->tableWidget->setItem(row, col, new QTableWidgetItem());
             }
        }
    }

    if (bNextRow) {
        // This means we wrapped around the cols
        row++;

        if (row > (_content_rows - 1)) {
            // Need to add a new row
            // printf("Adding a new row, because r[%d] and c[%d] - last_column: %d\n", row, col, _content_last_column);

            ui->tableWidget->insertRow ( ui->tableWidget->rowCount() );
            _content_rows++;
            _content_last_column = col;

            // Add all the COLUMNS to the new ROW
            for (int j = COLUMN_OFFSET; j <= COLUMN_ASCII_DATA; j++) {
                item = this->ui->tableWidget->item(row, j);
                if (item == NULL) {
                    ui->tableWidget->setItem(row, j, new QTableWidgetItem());
                }
            }
        }


        // update r
        *r = row;
    }
}

void MainWindow::on_tableWidget_ctrlCPressed(QModelIndexList selected)
{
    QString str;
    QClipboard *clipboard = QGuiApplication::clipboard();
    for (auto it = selected.begin(); it != selected.end(); ++it) {
        str += it->data().toString() + " ";
    }
    str = str.mid(0, str.length() - 1);
    clipboard->setText(str);
}


void MainWindow::on_tableWidget_ctrlVPressed(int r, int c)
{
    if (c == COLUMN_OFFSET || c == COLUMN_ASCII_DATA) {
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    QString text          = clipboard->text();

    /* The following regex expects to have a single match */
    // Ex: 23 23 23 65 6E 6E 20 34 2E 78 3A 20
    // Other input will have multiple matches:
    // Ex: AAAAAAA -> multiple matches from the regex.
    // Other option is to not have matches at all
    // Ex: XZZXZXZXZX
    // So the later two cases are the ones that we will treat differently.
    // If there is a single match, we know we have the same format we get from CTRL+C.
    // If there are multiple matches or no matches at all, then we treat it as arbitrary data

    QRegularExpressionMatch match = rx_copypaste.match(text);

    bool bExpectedFormat = false;
    if (match.hasMatch()) {
        // If has match and is the first match and is the same size as what is in the clipboard
        // then we have a winner
        QString str = match.capturedTexts().at(0);
        if (str.length() == text.length()) {
            bExpectedFormat = true;
            //QMessageBox::warning(this, "Warning", "YAY");
            int cur_row = r;
            int cur_col = c;
            QStringList hexbyte_list = text.split(" ");
            for (int i = 0 ; i < hexbyte_list.size(); i++) {
                QString hexbyte_str = hexbyte_list.at(i);
                QTableWidgetItem *item = ui->tableWidget->item(cur_row, cur_col);
                item->setText(hexbyte_str.toUpper());
                item->setTextAlignment(Qt::AlignCenter);
                next_hex_cell(&cur_row, &cur_col);
            }
        }
    }

    if (bExpectedFormat == false) {
        // There was no match before
        // Treat data as new input (arbitrary data that needs to be converted to hex-bytes)
        QByteArray new_data = text.toUtf8();
        int cur_row = r;
        int cur_col = c;
        int j = 0;
        while (j < new_data.size()) {
            unsigned char b = (unsigned char) new_data.at(j);
            //printf("Setting val to r[%d]c[%d] - max-rows: %d - last_col: %d\n", cur_row, cur_col, _content_rows, _content_last_column);
            QTableWidgetItem *item = ui->tableWidget->item(cur_row, cur_col);
            QString hexbyte_str;
            item->setText(hexbyte_str.sprintf("%02X", b));
            item->setTextAlignment(Qt::AlignCenter);
            j++;
            if (j == new_data.size()) {
                break;
            }
            next_hex_cell(&cur_row, &cur_col);
        }
    }

    re_load_ascii_area();
    find_strings_in_data(_content);
    set_strings_bg_color();

}


void MainWindow::on_actionSave_As_triggered()
{
    reload_content();

    QString filename = QFileDialog::getSaveFileName(this, "Save as");
    currentFile = filename;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot write file: " + file.errorString());
    }

    setWindowTitle(filename);

    file.write(_content);

    file.close();
}


void MainWindow::on_actionSave_triggered()
{
    reload_content();

    QFile file(currentFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot write file: " + file.errorString());
    }

    file.write(_content);

    file.close();
}


void MainWindow::on_actionClose_triggered()
{
    _content.clear();
    this->_content_last_column = 0;
    this->_content_rows        = 0;

    this->ui->tableWidget->clearContents();
}


void MainWindow::on_actionCopy_triggered()
{
    on_tableWidget_ctrlCPressed(this->ui->tableWidget->get_selected_indexes());
}


void MainWindow::on_actionPaste_triggered()
{
    on_tableWidget_ctrlVPressed(ui->tableWidget->currentRow(), ui->tableWidget->currentColumn());
}


void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

