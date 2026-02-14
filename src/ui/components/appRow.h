#pragma once
#ifndef APPROW_H
#define APPROW_H

#include "core/apps/readApps.h"
#include <QLabel>
#include <QWidget>

class AppRow : public QWidget
{
  Q_OBJECT

public:
  DesktopApp app;
  explicit AppRow(QWidget *parent = nullptr, const DesktopApp &app = DesktopApp());
  // expose hooks for pin/hide UI
  void setPinned(bool pinned);
  void setHiddenFlag(bool hidden);

private:
  const unsigned int ICON_SIZE = 24;
  const unsigned int TEXT_SIZE = 16;

  static const QString rowStyle;
  static const QString iconStyle;
  static const QString nameStyleTemplate;
};

#endif
