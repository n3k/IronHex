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

    this->table1_ctx.table = this->ui->tableWidget;
    this->table2_ctx.table = this->ui->tableWidget_2;

    this->ui->tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    this->ui->tableWidget_2->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    ItemDelegate *itDelegate = new  ItemDelegate;
    itDelegate->set_main_window(this);
    ui->tableWidget->setItemDelegate(itDelegate);

    ItemDelegate *itDelegate2 = new  ItemDelegate;
    itDelegate2->set_main_window(this);
    ui->tableWidget_2->setItemDelegate(itDelegate2);

    ui->tableWidget_2->resizeColumnsToContents();

//    connect(view1->horizontalScrollBar(), SIGNAL(valueChanged(int)), view2->horizontalScrollBar(), SLOT(setValue(int)));
//    connect(view2->horizontalScrollBar(), SIGNAL(valueChanged(int)), view1->horizontalScrollBar(), SLOT(setValue(int)));

      connect(this->ui->verticalScrollBar, SIGNAL(valueChanged(int)), this->ui->tableWidget->verticalScrollBar(), SLOT(setValue(int)));
      connect(this->ui->verticalScrollBar, SIGNAL(valueChanged(int)), this->ui->tableWidget_2->verticalScrollBar(), SLOT(setValue(int)));

      connect(this->ui->tableWidget->verticalScrollBar(),
              SIGNAL(rangeChanged(int, int)),
              this->ui->verticalScrollBar, SLOT(setRange(int, int)));
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

void MainWindow::reload_content(struct TableContext *t_ctx) {
    t_ctx->_content.clear();

    QString text = "";
    for(int i = 0 ; i < t_ctx->_content_rows; i++) {
        for (int j = COLUMN_FIRST_HEX_BYTE; j <= COLUMN_LAST_HEX_BYTE; j++) {

            QTableWidgetItem *item = t_ctx->table->item(i, j);

            if (i == (t_ctx->_content_rows - 1) && j > t_ctx->_content_last_column) {
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
            t_ctx->_content.push_back(v & 0xff);
        }
    }
}

/// This method only reloads the ascii-strings portion of the table
void MainWindow::re_load_ascii_area(struct TableContext *t_ctx) {
    // Get the bytes from the current hexdump and reload
    reload_content(t_ctx);

    size_t text_len = t_ctx->_content.length();
    QVector<QByteArray> data_vec;

    size_t p = 0;
    QByteArray str;
    while (p < text_len) {
        size_t rem = text_len - p;
        if (rem < 16) {
            str = t_ctx->_content.mid(p, rem);
        } else {
            str = t_ctx->_content.mid(p, 16);
        }

        data_vec.push_back(str);
        p += 16;
    }

    for(int i = 0 ; i < t_ctx->_content_rows; i++) {
        QByteArray data_line = data_vec.at(i);

        QTableWidgetItem *item = t_ctx->table->item(i, COLUMN_ASCII_DATA);
        //printf("Reloading ascii row: %d - %p\n", i, item);
        item->setText(get_non_ascii_chars_version(data_line));
        item->setBackgroundColor(Qt::lightGray);
        item->setFlags(Qt::ItemIsEnabled);
    }
}


/// This method will load or reload the entire content of the table
/// based on the input bytearray
void MainWindow::re_load_table_content(QByteArray &text, struct TableContext *t_ctx) {

    t_ctx->table->clearContents();

    size_t text_len = text.size();
    size_t lines_16 = text_len >> 4;
    lines_16 += ((text_len & 0b1111) != 0) ? 1: 0;
    size_t needed_rows = lines_16;
    t_ctx->_content_rows = needed_rows;

    const size_t column_num = t_ctx->table->columnCount();

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
    int additional_rows = needed_rows - t_ctx->table->rowCount();

    while (additional_rows > 0) {
        t_ctx->table->insertRow ( t_ctx->table->rowCount() );
        additional_rows--;
    }

    size_t offset = 0;
    for(size_t i = 0; i < needed_rows; i++) {

        QByteArray data_line = data_vec.at(i);

        for(size_t j = 0; j < column_num; j++)
        {

            QTableWidgetItem *item = t_ctx->table->item(i, j);
            if(!item) {
                item = new QTableWidgetItem();
                t_ctx->table->setItem(i, j, item);
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
                    t_ctx->_content_last_column = j;
                }
            }
        }
    }

    t_ctx->table->resizeColumnsToContents();

    offset_vec.clear();
    data_vec.clear();
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

void MainWindow::find_strings_in_data(QByteArray &data, struct TableContext *t_ctx) {
    t_ctx->string_map.clear();

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
                t_ctx->string_map[i] = str_len;
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
    this->table1_ctx._content.clear();
    this->table1_ctx.table->setRowCount(0);

    bInitialLoadDone = false;

    QString filename = QFileDialog::getOpenFileName(this, "Open the file");
    currentFile1 = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
    }

    setWindowTitle(filename);

    QByteArray text = file.readAll();

    // find strings
    find_strings_in_data(text, &this->table1_ctx);

    // fill table
    re_load_table_content(text, &this->table1_ctx);
    bInitialLoadDone = true;

    // set color for strings
    set_strings_bg_color(&this->table1_ctx);


    file.close();
}

void MainWindow::set_strings_bg_color(struct TableContext *t_ctx) {

    for (auto it = t_ctx->string_map.begin(); it != t_ctx->string_map.end(); ++it) {
        int cur_row = 0;
        int cur_col = 0;
        map_offset_to_cell(it.key(), &cur_row, &cur_col);

        for (int i = 0; i < it.value(); i++) {
            //QString str;
            //QMessageBox::warning(this, "Warning", str.sprintf("r[%d]c[%d]", cur_row, cur_col));
            QTableWidgetItem *item = t_ctx->table->item(cur_row, cur_col);
            item->setBackgroundColor(Qt::yellow);
            next_hex_cell(&cur_row, &cur_col, t_ctx);
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
void MainWindow::next_hex_cell(int *r, int *c, struct TableContext *t_ctx) {

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
    if (row == (t_ctx->_content_rows - 1)) {

        // If we re on the last ROW and our increased COLUMN is greater than what we tracked before
        // Then increase the _content_last_column (to col)
        if (col > t_ctx->_content_last_column) {
             t_ctx->_content_last_column = col;
             // Add new item
             item = t_ctx->table->item(row, col);
             if (item == NULL) {
                 QMessageBox::warning(this, "Warning", "adding new COL");
                 t_ctx->table->setItem(row, col, new QTableWidgetItem());
             }
        }
    }

    if (bNextRow) {
        // This means we wrapped around the cols
        row++;

        if (row > (t_ctx->_content_rows - 1)) {
            // Need to add a new row
            // printf("Adding a new row, because r[%d] and c[%d] - last_column: %d\n", row, col, _content_last_column);

            t_ctx->table->insertRow ( t_ctx->table->rowCount() );
            t_ctx->_content_rows++;
            t_ctx->_content_last_column = col;

            // Add all the COLUMNS to the new ROW
            for (int j = COLUMN_OFFSET; j <= COLUMN_ASCII_DATA; j++) {
                item = t_ctx->table->item(row, j);
                if (item == NULL) {
                    t_ctx->table->setItem(row, j, new QTableWidgetItem());
                }
            }
        }


        // update r
        *r = row;
    }
}


void MainWindow::handle_ctrl_c(QModelIndexList selected) {
    QString str;
    QClipboard *clipboard = QGuiApplication::clipboard();
    for (auto it = selected.begin(); it != selected.end(); ++it) {
        str += it->data().toString() + " ";
    }
    str = str.mid(0, str.length() - 1);
    clipboard->setText(str);
}

void MainWindow::on_tableWidget_ctrlCPressed(QModelIndexList selected)
{
    handle_ctrl_c(selected);
}

void MainWindow::on_tableWidget_2_ctrlCPressed(QModelIndexList selected)
{
    handle_ctrl_c(selected);
}


void MainWindow::handle_ctrl_v(int r, int c, struct TableContext *t_ctx) {
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
                QTableWidgetItem *item = t_ctx->table->item(cur_row, cur_col);
                item->setText(hexbyte_str.toUpper());
                item->setTextAlignment(Qt::AlignCenter);
                next_hex_cell(&cur_row, &cur_col, t_ctx);
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
            QTableWidgetItem *item = t_ctx->table->item(cur_row, cur_col);
            QString hexbyte_str;
            item->setText(hexbyte_str.sprintf("%02X", b));
            item->setTextAlignment(Qt::AlignCenter);
            j++;
            if (j == new_data.size()) {
                break;
            }
            next_hex_cell(&cur_row, &cur_col, t_ctx);
        }
    }

    re_load_ascii_area(t_ctx);
    find_strings_in_data(t_ctx->_content, t_ctx);
    set_strings_bg_color(t_ctx);
}

void MainWindow::on_tableWidget_ctrlVPressed(int r, int c)
{
    this->handle_ctrl_v(r, c, &this->table1_ctx);
}

void MainWindow::on_tableWidget_2_ctrlVPressed(int r, int c)
{
    this->handle_ctrl_v(r, c, &this->table2_ctx);
}


void MainWindow::on_actionSave_As_triggered()
{
    reload_content(&this->table1_ctx);

    QString filename = QFileDialog::getSaveFileName(this, "Save as");
    currentFile1 = filename;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot write file: " + file.errorString());
    }

    setWindowTitle(filename);

    file.write(this->table1_ctx._content);

    file.close();
}


void MainWindow::on_actionSave_triggered()
{
    reload_content(&this->table1_ctx);

    QFile file(currentFile1);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot write file: " + file.errorString());
    }

    file.write(this->table1_ctx._content);

    file.close();
}


void MainWindow::on_actionClose_triggered()
{
    this->table1_ctx._content.clear();
    this->table2_ctx._content.clear();
    this->table1_ctx.string_map.clear();
    this->table2_ctx.string_map.clear();
    this->table1_ctx._content_rows = 0;
    this->table2_ctx._content_rows = 0;
    this->table1_ctx._content_last_column = 0;
    this->table2_ctx._content_last_column = 0;


    this->table1_ctx.table->clearContents();
    this->table2_ctx.table->clearContents();

    bInitialLoadDone = false;
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


void MainWindow::on_actionLoad2_triggered()
{
    if (!this->bInitialLoadDone) {
        QMessageBox::warning(this, "Warning", "Primary table was not filled in!");
        return;
    }

    this->table2_ctx._content.clear();
    this->table2_ctx.table->setRowCount(0);

    QString filename = QFileDialog::getOpenFileName(this, "Open the file");
    currentFile2 = filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
    }

    QByteArray text = file.readAll();

    // fill table
    re_load_table_content(text, &this->table2_ctx);


    do_diff();

    file.close();
}


void MainWindow::do_diff() {
    reload_content(&this->table1_ctx);
    reload_content(&this->table2_ctx);

    int len1 = this->table1_ctx._content.size();
    int len2 = this->table2_ctx._content.size();

    int len_to_cmp = len1 <= len2 ? len1 : len2;

    int cur_row = 0;
    int cur_col = 0;

    for (int i = 0; i < len_to_cmp; i++) {
        if (this->table1_ctx._content.at(i) != this->table2_ctx._content.at(i)) {
            map_offset_to_cell(i, &cur_row, &cur_col);

            QTableWidgetItem *item = table2_ctx.table->item(cur_row, cur_col);
            item->setBackgroundColor(Qt::red);
        }
    }



}

