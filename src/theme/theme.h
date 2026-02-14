#pragma once
#include <QString>
#include <QColor>
#include <QFont>
#include <QMap>
#include <QStringList>

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
  // Load a Qt stylesheet (.qss) from disk; returns true if loaded
  bool loadStyleSheet(const QString &path);
  // Return loaded stylesheet (may be empty)
  QString styleSheet() const;
  // Return a built-in stylesheet by name ("dark", "light", "solarized")
  static QString builtinStyle(const QString &name);
  // List of available built-in theme names
  static QStringList builtinNames();
  // Generate a QSS string from a Theme struct that targets our app object names
  static QString qssForTheme(const Theme &t);
  // Return a Theme struct for one of the built-in theme names
  static Theme themeForBuiltin(const QString &name);
  // Load a QSS from a file path (returns empty on failure)
  static QString loadQssFile(const QString &path);
  // Apply a stylesheet and theme values to the application and a root widget
  static void applyStyle(QApplication &app, QWidget *root, const QString &qss, const Theme &t);
  const Theme &currentTheme() const;
  static QString defaultConfigPath();
  static QString defaultStylePath();
  // Directory where builtin QSS files are written for user inspection/customization
  static QString defaultThemesDir();

private:
  Theme theme;
  void setDefaults();
  // Ensure builtin QSS files are present on disk (~/.config/dlauncher/themes)
  void ensureBuiltinQssFiles();
};
