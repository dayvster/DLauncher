
#include "LockedLineEdit.h"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QListWidget>

LockedLineEdit::LockedLineEdit(QWidget *parent, QListWidget *list)
    : QLineEdit(parent), listWidget(list) {}

// Keep focus on input
void LockedLineEdit::focusOutEvent(QFocusEvent *event) {
  this->setFocus();
  QLineEdit::focusOutEvent(event);
}

// Handle key presses
void LockedLineEdit::keyPressEvent(QKeyEvent *event) {
  if (!listWidget || listWidget->count() == 0) {
    QLineEdit::keyPressEvent(event);
    return;
  }

  int current = listWidget->currentRow();

  switch (event->key()) {
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
    current = (current + 1) % listWidget->count(); // wrap around
    listWidget->setCurrentRow(current);
    listWidget->scrollToItem(listWidget->currentItem());
    break;

  case Qt::Key_Backtab: // Shift+Tab
    current = (current - 1 + listWidget->count()) %
              listWidget->count(); // wrap backward
    listWidget->setCurrentRow(current);
    listWidget->scrollToItem(listWidget->currentItem());
    break;

  default:
    QLineEdit::keyPressEvent(event); // normal typing
  }
}
