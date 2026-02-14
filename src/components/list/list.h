
#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListWidget>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>

class ListView : public QWidget {
  Q_OBJECT

public:
  QListWidget *listWidget = nullptr;

  explicit ListView(QWidget *parent = nullptr);
  ListView(QWidget *parent, const QList<QWidget *> &rows);
  ~ListView();

  void addRow(QWidget *row);
  void addRows(const QList<QWidget *> &rows);
  void removeRow(int index);
  void removeAllRows();
  void setRows(const QList<QWidget *> &rows);

  unsigned int count = 0;

private:
  QStringListModel *model = nullptr;
};

#endif
