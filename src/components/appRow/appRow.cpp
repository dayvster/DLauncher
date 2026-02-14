#include "appRow.h"
#include "appRow.h"
#include <QBrush>
#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QString>

const QString AppRow::rowStyle = R"(
    /* kept for reference; prefer global QSS from ThemeManager */
    AppRow {
        background-color: rgba(255, 255, 255, 10);
        border-radius: 8px;
    }
    AppRow:hover {
        background-color: rgba(155, 255, 255, 30);
    }
)";

const QString AppRow::iconStyle = R"(
    QLabel {
        color: white;
        font-size: 18px;
    }
)";

const QString AppRow::nameStyleTemplate = R"(
    QLabel {
        color: white;
        font-size: %1px;
    }
)";

AppRow::AppRow(QWidget *parent, const DesktopApp &app) : QWidget(parent), app(app)
{
  // Use a stable object name so theme QSS selectors can target rows
  this->setObjectName("AppRow");
  this->setToolTip(QString::fromStdString(app.comment.value_or("")));
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  // Prefer global QSS where possible; only keep minimal inline styling that
  // doesn't conflict with the global stylesheet. Move row visuals into the
  // ThemeManager-generated QSS so themes can control appearance consistently.
  // Remove the fixed inline stylesheet to allow application-level QSS to take
  // effect.
  // this->setStyleSheet(rowStyle);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(8, 4, 8, 4);
  layout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

  QLabel *iconLabel = new QLabel(this);

  bool iconSet = false;
  if (app.icon)
  {
    QString iconName = QString::fromStdString(app.icon.value());
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull())
    {
      iconLabel->setPixmap(icon.pixmap(ICON_SIZE, ICON_SIZE));
      iconSet = true;
    }
  }

  if (!iconSet)
  {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QBrush(QColor(100, 100, 100)));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(0, 0, ICON_SIZE, ICON_SIZE, 4, 4);

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(ICON_SIZE / 2);
    painter.setFont(font);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.drawText(pixmap.rect(), Qt::AlignCenter,
                     QString::fromStdString(app.name.substr(0, 1)));

    painter.end();
    iconLabel->setPixmap(pixmap);
  }

  iconLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  layout->addWidget(iconLabel);

  QLabel *nameLabel = new QLabel(QString::fromStdString(app.name), this);
  // Rely on the global stylesheet for font size/color. Keep a minimal local
  // override only if the theme hasn't provided a rule (fallback to default).
  nameLabel->setStyleSheet(nameStyleTemplate.arg(TEXT_SIZE + 4));
  nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  layout->addWidget(nameLabel);

  layout->addStretch();

  this->setLayout(layout);
}
