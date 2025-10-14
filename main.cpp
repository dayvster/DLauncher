#include <fstream>

#include "LockedLineEdit.h"
#include "list/list.h"
#include "listeners/kb.h"
#include "src/components/appRow/appRow.h"
#include "src/utils/readApps.h"
#include "utils.h"
#include "src/utils/theme.h"
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>
#include <qprocess.h>
#include <qsizepolicy.h>
#include "src/utils/utils.h"
#include "src/utils/debug.h"

Qt::WindowFlags devFlags()
{
  return Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
}

int main(int argc, char *argv[])
// ...existing code...
{
  // Frequency map: app exec string -> launch count (persistent)
  std::string configDir = std::string(getenv("HOME")) + "/.config";
  std::string freqPath = configDir + "/dlauncher_freq.txt";
  // Ensure config dir exists
  std::filesystem::create_directories(configDir);
  // Ensure file exists and is readable/writable
  {
    std::ofstream f(freqPath, std::ios::app);
    f.close();
  }
  std::map<std::string, int> appFrequency;
  {
    std::ifstream f(freqPath);
    std::string line;
    while (std::getline(f, line))
    {
      auto pos = line.find(':');
      if (pos != std::string::npos)
      {
        std::string key = line.substr(0, pos);
        try
        {
          int val = std::stoi(line.substr(pos + 1));
          appFrequency[key] = val;
        }
        catch (...)
        {
        }
      }
    }
  }
  Debug::log("Debug run string: " + Debug::generateRandomString());
  AppReader appReader;
  QApplication app(argc, argv);
  ThemeManager themeManager;
  const Theme &theme = themeManager.currentTheme();
  GlobalEventListener globalKbListener(app);
  QProcess *process = new QProcess();

  appReader.LoadApps();

  std::string searchTerm = "";

  globalKbListener.registerKeyCallback(Qt::Key_Escape, [&]()
                                       {
    std::cout << "Escape key pressed, exiting..." << std::endl;
    app.quit();
    exit(0);
    return 0; });

  QWidget window;
  window.setWindowFlags(devFlags());
  window.setFixedSize(theme.windowWidth, theme.windowHeight);
  window.setAttribute(Qt::WA_TranslucentBackground);
  QPalette pal = window.palette();
  pal.setColor(QPalette::Window, theme.backgroundColor);
  window.setPalette(pal);
  window.setAutoFillBackground(true);
  // Set window position if specified, otherwise center
  if (theme.windowPosX >= 0 && theme.windowPosY >= 0)
  {
    window.move(theme.windowPosX, theme.windowPosY);
  }
  else
  {
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - theme.windowWidth) / 2;
    int y = (screenGeometry.height() - theme.windowHeight) / 2;
    window.move(x, y);
  }

  ListView *list = new ListView(&window);

  QVBoxLayout *layout = new QVBoxLayout(&window);

  LockedLineEdit *input = new LockedLineEdit(&window, list->listWidget);
  input->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            border: 2px solid %2;
            border-radius: 10px;
            padding: 10px;
            color: %3;
            font-size: %4px;
        }
    )")
                           .arg(theme.inputBackground.name(QColor::HexArgb))
                           .arg(theme.inputBorder.name(QColor::HexArgb))
                           .arg(theme.textColor.name(QColor::HexArgb))
                           .arg(theme.font.pointSize()));
  input->setFont(theme.font);
  input->setPlaceholderText("Search...");
  input->setFocus();

  layout->addWidget(input);

  std::vector<DesktopApp> allApps = appReader.ReadDesktopApps(64, "");

  for (const auto &app : allApps)
  {
    list->addRow(new AppRow(list, app));
  }
  // Optionally, you can pass theme to AppRow for per-row theming if needed

  QObject::connect(list->listWidget, &QListWidget::currentRowChanged, [&](int row)
                   {
    if (row >= 0 && row < list->listWidget->count()) {
      QListWidgetItem *item = list->listWidget->item(row);
      AppRow *appRow = dynamic_cast<AppRow *>(list->listWidget->itemWidget(item));
      if (appRow) {
        std::cout << "Exec: " << appRow->app.exec << std::endl;
      }
    } });

  layout->addWidget(list);

  QObject::connect(input, &QLineEdit::textChanged, [&](const QString &text)
                   {
    std::string search = text.toStdString();
    list->listWidget->clear();
    if (search.empty()) {
      std::vector<DesktopApp> allApps = appReader.ReadDesktopApps(64, "");
      std::sort(allApps.begin(), allApps.end(), [&](const DesktopApp &a, const DesktopApp &b) {
        int fa = appFrequency[a.exec];
        int fb = appFrequency[b.exec];
        if (fa == fb) {
          // Alphabetical tiebreaker
          return a.name < b.name;
        }
        // Zero frequency always at the bottom
        if (fa == 0) return false;
        if (fb == 0) return true;
        return fa > fb;
      });
      for (const auto &app : allApps) {
        list->addRow(new AppRow(list, app));
      }
      return;
    }

    // Only app search: fuzzy match apps
    std::vector<DesktopApp> filteredApps;
    for (const auto &app : appReader.GetAllApps()) {
      if (similarity(search, app.name) > 0.5 || contains(app.name, search, false) != std::string::npos) {
        filteredApps.push_back(app);
      }
    }
    // Sort by frequency (most-run first)
    std::sort(filteredApps.begin(), filteredApps.end(), [&](const DesktopApp &a, const DesktopApp &b) {
      int fa = appFrequency[a.exec];
      int fb = appFrequency[b.exec];
      if (fa == fb) {
        return a.name < b.name;
      }
      if (fa == 0) return false;
      if (fb == 0) return true;
      return fa > fb;
    });
    for (const auto &app : filteredApps) {
      list->addRow(new AppRow(list, app));
    } });

  QObject::connect(input, &QLineEdit::returnPressed, [&]()
                   {
    int row = list->listWidget->currentRow();
    if (row >= 0 && row < list->listWidget->count()) {
      QListWidgetItem *item = list->listWidget->item(row);
      AppRow *appRow = dynamic_cast<AppRow *>(list->listWidget->itemWidget(item));
      if (appRow) {
        auto [program, args] = parseExecCommand(QString::fromStdString(appRow->app.exec));
        if (program.isEmpty()) return;
        std::cout << "Launching: " << program.toStdString();
        for (const auto &arg : args) std::cout << " " << arg.toStdString();
        std::cout << std::endl;
  // Increment frequency count and persist
  appFrequency[appRow->app.exec]++;
  {
    appFrequency[appRow->app.exec]++;
    std::ofstream f(freqPath);
    for (const auto& [key, val] : appFrequency) {
      if (val > 0) {
        f << key << ":" << val << "\n";
      }
    }
  }
  QProcess::startDetached(program, args);
  app.quit();
      }
    } });

  QRect screenGeometry = QApplication::primaryScreen()->geometry();
  int x = (screenGeometry.width() - window.width()) / 2;
  int y = (screenGeometry.height() - window.height()) / 2;
  window.move(x, y);

  window.show();

  return app.exec();
}
