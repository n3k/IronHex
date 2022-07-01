#include "irontable.h"
#include <QMessageBox>


void IronTable::spacePressed(int r, int c) {
    QString str;
    QMessageBox::warning(this, "Warning", str.sprintf("space pressed on X[%d]Y[%d]", r, c));
}
