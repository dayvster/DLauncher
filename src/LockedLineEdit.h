
#ifndef LOCKEDLINEEDIT_H
#define LOCKEDLINEEDIT_H

#include <QLineEdit>

class QListWidget;

class LockedLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  explicit LockedLineEdit(QWidget *parent = nullptr,
                          QListWidget *list = nullptr);

protected:
  void focusOutEvent(QFocusEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  QListWidget *listWidget;
};

#endif
