#pragma once
#include <QString>
#include <QColor>
#include <QFont>
#include <QMap>

struct Theme
{
  QColor backgroundColor;
  QColor borderColor;
  QColor textColor;
  QColor highlightColor;
  QColor inputBackground;
  QColor inputBorder;
  QFont font;
  int fontSize = 16;
  int borderRadius = 10;
  int padding = 10;
  int windowWidth = 500;
  int windowHeight = 300;
  int windowPosX = -1;
  int windowPosY = -1;
  QColor selectionColor;
  QColor rowBackground;
  QColor rowHover;
};

class ThemeManager
{
public:
  ThemeManager();
  bool loadTheme(const QString &path);
  const Theme &currentTheme() const;
  static QString defaultConfigPath();

private:
  Theme theme;
  void setDefaults();
};
