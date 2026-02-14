
#include "list.h"

QString StyleSheet = R"(
        QListWidget {
            background-color: rgba(0, 0, 0, 0);
            border: 2px solid rgba(0, 0, 0, 0);
            padding: 0px;
            width: 100%;
            color: white;
            font-size: 16px;
        }
        QScrollBar:vertical {
              border: none;
              background: rgba(0, 0, 0, 50);
              width: 10px;
              margin: 0px 0px 0px 0px;
              border-radius: 5px;
          }
        QListWidget::item {
        }
        QListWidget::item:selected {
        }
    )";

ListView::ListView(QWidget *parent) : QWidget(parent) {
  listWidget = new QListWidget(this);
  listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // Remove local stylesheet so the global/app stylesheet can control visuals.
  // listWidget->setStyleSheet(StyleSheet);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(listWidget);

  setLayout(layout);
}

ListView::ListView(QWidget *parent, const QList<QWidget *> &rows)
    : QWidget(parent) {
  listWidget = new QListWidget(this);
  // listWidget->setStyleSheet(StyleSheet);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(listWidget);
  setLayout(layout);
  addRows(rows);
  count = rows.size();
}

ListView::~ListView() { delete model; }

void ListView::addRow(QWidget *row) {
  QListWidgetItem *item = new QListWidgetItem();
  item->setSizeHint(row->sizeHint());
  listWidget->addItem(item);
  listWidget->setItemWidget(item, row);
  count++;
}

void ListView::removeAllRows() {
  if (listWidget) {
    listWidget->clear();
    count = 0;
  }
}

void ListView::setRows(const QList<QWidget *> &rows) {
  delete listWidget;

  listWidget = new QListWidget(this);
  layout()->addWidget(listWidget);

  for (QWidget *row : rows) {
    addRow(row);
  }
  count = rows.size();
}

void ListView::addRows(const QList<QWidget *> &rows) {
  for (QWidget *row : rows) {
    addRow(row);
  }
}

void ListView::removeRow(int index) {
  if (listWidget && index >= 0 && index < listWidget->count()) {
    delete listWidget->takeItem(index);
    count--;
  }
}
