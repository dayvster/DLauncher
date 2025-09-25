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
    if (key == "background")
    {
      theme.backgroundColor = QColor(value);
      anySet = true;
    }
    else if (key == "border")
    {
      theme.borderColor = QColor(value);
      anySet = true;
    }
    else if (key == "text")
    {
      theme.textColor = QColor(value);
      anySet = true;
    }
    else if (key == "highlight")
    {
      theme.highlightColor = QColor(value);
      anySet = true;
    }
    else if (key == "input_bg")
    {
      theme.inputBackground = QColor(value);
      anySet = true;
    }
    else if (key == "input_border")
    {
      theme.inputBorder = QColor(value);
      anySet = true;
    }
    else if (key == "font")
    {
      theme.font = QFont(value, theme.fontSize);
      anySet = true;
    }
    else if (key == "font_size")
    {
      theme.fontSize = value.toInt();
      theme.font.setPointSize(theme.fontSize);
      anySet = true;
    }
    else if (key == "border_radius")
    {
      theme.borderRadius = value.toInt();
      anySet = true;
    }
    else if (key == "padding")
    {
      theme.padding = value.toInt();
      anySet = true;
    }
    else if (key == "selection")
    {
      theme.selectionColor = QColor(value);
      anySet = true;
    }
    else if (key == "row_bg")
    {
      theme.rowBackground = QColor(value);
      anySet = true;
    }
    else if (key == "row_hover")
    {
      theme.rowHover = QColor(value);
      anySet = true;
    }
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
