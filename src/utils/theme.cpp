#include "theme.h"
#include <QFile>
#include <QTextStream>
#include <QDir>

ThemeManager::ThemeManager()
{
  setDefaults();
  loadTheme(defaultConfigPath());
}

void ThemeManager::setDefaults()
{
  theme.backgroundColor = QColor(30, 30, 30, 220);
  theme.borderColor = QColor(51, 51, 51, 255);
  theme.textColor = QColor(255, 255, 255, 255);
  theme.highlightColor = QColor(0, 188, 212, 100);
  theme.inputBackground = QColor(34, 34, 34, 255);
  theme.inputBorder = QColor(68, 68, 68, 255);
  theme.font = QFont("FiraCode", 16);
  theme.fontSize = 16;
  theme.borderRadius = 10;
  theme.padding = 10;
  theme.selectionColor = QColor(0, 188, 212, 80);
  theme.rowBackground = QColor(255, 255, 255, 10);
  theme.rowHover = QColor(155, 255, 255, 30);
  theme.windowWidth = 500;
  theme.windowHeight = 300;
  theme.windowPosX = -1;
  theme.windowPosY = -1;
}

bool ThemeManager::loadTheme(const QString &path)
{
  QFile file(path);
  if (!file.exists())
    return false;
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;
  QTextStream in(&file);
  bool anySet = false;
  using Setter = std::function<void(const QString &)>;
  QMap<QString, Setter> setters = {
      {"background", [&](const QString &v) { theme.backgroundColor = QColor(v); anySet = true; }},
      {"border", [&](const QString &v) { theme.borderColor = QColor(v); anySet = true; }},
      {"text", [&](const QString &v) { theme.textColor = QColor(v); anySet = true; }},
      {"highlight", [&](const QString &v) { theme.highlightColor = QColor(v); anySet = true; }},
      {"input_bg", [&](const QString &v) { theme.inputBackground = QColor(v); anySet = true; }},
      {"input_border", [&](const QString &v) { theme.inputBorder = QColor(v); anySet = true; }},
      {"font", [&](const QString &v) { theme.font = QFont(v, theme.fontSize); anySet = true; }},
      {"font_size", [&](const QString &v) { theme.fontSize = v.toInt(); theme.font.setPointSize(theme.fontSize); anySet = true; }},
      {"border_radius", [&](const QString &v) { theme.borderRadius = v.toInt(); anySet = true; }},
      {"padding", [&](const QString &v) { theme.padding = v.toInt(); anySet = true; }},
      {"window_width", [&](const QString &v) { theme.windowWidth = v.toInt(); anySet = true; }},
      {"window_height", [&](const QString &v) { theme.windowHeight = v.toInt(); anySet = true; }},
      {"window_pos_x", [&](const QString &v) { theme.windowPosX = v.toInt(); anySet = true; }},
      {"window_pos_y", [&](const QString &v) { theme.windowPosY = v.toInt(); anySet = true; }},
      {"selection", [&](const QString &v) { theme.selectionColor = QColor(v); anySet = true; }},
      {"row_bg", [&](const QString &v) { theme.rowBackground = QColor(v); anySet = true; }},
      {"row_hover", [&](const QString &v) { theme.rowHover = QColor(v); anySet = true; }}
  };
  while (!in.atEnd())
  {
    QString line = in.readLine().trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      continue;
    auto parts = line.split('=');
    if (parts.size() != 2)
      continue;
    QString key = parts[0].trimmed();
    QString value = parts[1].trimmed();
    if (setters.contains(key))
      setters[key](value);
  }
  if (!anySet)
    setDefaults(); // If nothing was set, use sane defaults
  return true;
}

const Theme &ThemeManager::currentTheme() const
{
  return theme;
}

QString ThemeManager::defaultConfigPath()
{
  return QDir::homePath() + "/.config/dlauncher/theme";
}
