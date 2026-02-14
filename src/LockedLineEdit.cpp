
#include "LockedLineEdit.h"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QListWidget>

LockedLineEdit::LockedLineEdit(QWidget *parent, QListWidget *list)
    : QLineEdit(parent), listWidget(list) {}

<<<<<<< HEAD
void LockedLineEdit::focusOutEvent(QFocusEvent *event){
=======
void LockedLineEdit::focusOutEvent(QFocusEvent *event)
{
>>>>>>> origin/main
  this->setFocus();
  QLineEdit::focusOutEvent(event);
}

<<<<<<< HEAD
void LockedLineEdit::keyPressEvent(QKeyEvent *event) {
=======
void LockedLineEdit::keyPressEvent(QKeyEvent *event)
{
>>>>>>> origin/main
  if (!listWidget || listWidget->count() == 0)
  {
    QLineEdit::keyPressEvent(event);
    return;
  }

  int current = listWidget->currentRow();

  switch (event->key())
  {
  case Qt::Key_Up:
    current = std::max(0, current - 1);
    listWidget->setCurrentRow(current);
    listWidget->scrollToItem(listWidget->currentItem());
    break;

  case Qt::Key_Down:
    current = std::min(listWidget->count() - 1, current + 1);
    listWidget->setCurrentRow(current);
    listWidget->scrollToItem(listWidget->currentItem());
    break;

  case Qt::Key_Tab:
    current = (current + 1) % listWidget->count();
    listWidget->setCurrentRow(current);
    listWidget->scrollToItem(listWidget->currentItem());
    break;

  case Qt::Key_Backtab:
    current = (current - 1 + listWidget->count()) %
              listWidget->count();
    listWidget->setCurrentRow(current);
    listWidget->scrollToItem(listWidget->currentItem());
    break;

  default:
<<<<<<< HEAD
    QLineEdit::keyPressEvent(event);
=======
    QLineEdit::keyPressEvent(event); // normal typing
>>>>>>> origin/main
  }
}
