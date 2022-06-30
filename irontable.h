#ifndef IRONTABLE_H
#define IRONTABLE_H

#endif // IRONTABLE_H

#include <QTableWidget>
#include <QKeyEvent>

class IronTable:public QTableWidget
{
   Q_OBJECT
   public:
      IronTable(QWidget* parent=0):QTableWidget(parent){}

      QModelIndexList get_selected_indexes() {
          return this->selectedIndexes();
      };

   protected:
      void keyPressEvent(QKeyEvent *e)
      {
         if(e->key()==Qt::Key_Space)
         {
            emit spacePressed(this->currentRow(),this->currentColumn());
         }
         else if(e->key()==Qt::Key_C && e->modifiers() & Qt::ControlModifier)
         {
            emit ctrlCPressed(this->selectedIndexes());
         }
         else if(e->key()==Qt::Key_V && e->modifiers() & Qt::ControlModifier)
         {
            emit ctrlVPressed(this->currentRow(),this->currentColumn());
         }

         else { QTableWidget::keyPressEvent(e); }
      }
   signals:
      void spacePressed(int r, int c);
      void ctrlCPressed(QModelIndexList selected);
      void ctrlVPressed(int r, int c);
};
