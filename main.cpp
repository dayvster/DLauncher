#include <fstream>
<<<<<<< HEAD
#include <filesystem>

#include "LockedLineEdit.h"
#include "src/ui/components/list.h"
#include "src/core/listeners/kb.h"
#include "src/ui/components/appRow.h"
#include "src/core/apps/readApps.h"
#include "utils.h"
#include "src/theme/theme.h"
=======

#include "LockedLineEdit.h"
#include "list/list.h"
#include "listeners/kb.h"
#include "src/components/appRow/appRow.h"
#include "src/utils/readApps.h"
#include "utils.h"
#include "src/utils/theme.h"
>>>>>>> origin/main
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
<<<<<<< HEAD
#include "src/utils/json.hpp"
#include <QRegularExpression>
=======
>>>>>>> origin/main

Qt::WindowFlags devFlags()
{
  return Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
}

int main(int argc, char *argv[])
<<<<<<< HEAD
{
  std::string configDir = std::string(getenv("HOME")) + "/.config";
  std::string freqPath = configDir + "/dlauncher_freq.txt";
  std::filesystem::create_directories(configDir);
=======
// ...existing code...
{
  // Frequency map: app exec string -> launch count (persistent)
  std::string configDir = std::string(getenv("HOME")) + "/.config";
  std::string freqPath = configDir + "/dlauncher_freq.txt";
  // Ensure config dir exists
  std::filesystem::create_directories(configDir);
  // Ensure file exists and is readable/writable
>>>>>>> origin/main
  {
    std::ofstream f(freqPath, std::ios::app);
    f.close();
  }
<<<<<<< HEAD
  std::map<std::string, int> appFrequency = json_util::load_freq(freqPath);
=======
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
>>>>>>> origin/main
  Debug::log("Debug run string: " + Debug::generateRandomString());
  AppReader appReader;
  QApplication app(argc, argv);
  ThemeManager themeManager;
  const Theme &theme = themeManager.currentTheme();
  GlobalEventListener globalKbListener(app);
<<<<<<< HEAD

  // By default don't include NoDisplay/Hidden entries; allow override via CLI flag
  // New flag: --include-hidden (accept --show-hidden for backward compatibility)
  bool includeHidden = false;
  bool menuMode = false;
  std::vector<std::pair<std::string, std::string>> menuItems; // label, command
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "--include-hidden" || arg == "--show-hidden") includeHidden = true;
    else if (arg == "--menu-file" && i + 1 < argc) {
      menuMode = true;
      std::string path = argv[++i];
      std::ifstream mf(path);
      if (mf.is_open()) {
        std::string line;
        while (std::getline(mf, line)) {
          if (line.empty()) continue;
          // Support separators: "::", '\t', or '|'
          std::string label, cmd;
          auto pos = line.find("::");
          if (pos != std::string::npos) {
            label = line.substr(0, pos);
            cmd = line.substr(pos + 2);
          } else if ((pos = line.find('\t')) != std::string::npos) {
            label = line.substr(0, pos);
            cmd = line.substr(pos + 1);
          } else if ((pos = line.find('|')) != std::string::npos) {
            label = line.substr(0, pos);
            cmd = line.substr(pos + 1);
          } else {
            // whole line is command
            cmd = line;
            label = line;
          }
          if (!label.empty() && !cmd.empty()) {
            menuItems.emplace_back(label, cmd);
          }
        }
      }
    } else if (arg == "--menu-item" && i + 1 < argc) {
      menuMode = true;
      std::string pair = argv[++i];
      auto pos = pair.find("::");
      if (pos == std::string::npos) pos = pair.find('|');
      std::string label, cmd;
      if (pos != std::string::npos) {
        label = pair.substr(0, pos);
        cmd = pair.substr(pos + (pair[pos]==':'?2:1));
      } else {
        label = pair; cmd = pair;
      }
      if (!label.empty() && !cmd.empty()) menuItems.emplace_back(label, cmd);
    }
  }
  appReader.LoadApps(includeHidden);

  std::string searchTerm = "";

  globalKbListener.registerKeyCallback(Qt::Key_Escape, [&]() {
    app.quit();
  });
=======
  QProcess *process = new QProcess();

  appReader.LoadApps();

  std::string searchTerm = "";

  globalKbListener.registerKeyCallback(Qt::Key_Escape, [&]()
                                       {
    std::cout << "Escape key pressed, exiting..." << std::endl;
    app.quit();
    exit(0);
    return 0; });
>>>>>>> origin/main

  QWidget window;
  window.setWindowFlags(devFlags());
  window.setFixedSize(theme.windowWidth, theme.windowHeight);
  window.setAttribute(Qt::WA_TranslucentBackground);
  QPalette pal = window.palette();
  pal.setColor(QPalette::Window, theme.backgroundColor);
  window.setPalette(pal);
  window.setAutoFillBackground(true);
<<<<<<< HEAD
=======
  // Set window position if specified, otherwise center
>>>>>>> origin/main
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

<<<<<<< HEAD
  std::vector<DesktopApp> allApps;
  if (menuMode) {
    for (auto &p : menuItems) {
      DesktopApp a;
      a.name = p.first;
      a.exec = p.second;
      allApps.push_back(a);
    }
  } else {
    allApps = appReader.ReadDesktopApps(64, "");
  }

  for (const auto &app : allApps) {
    list->addRow(new AppRow(list, app));
  }
  
=======
  std::vector<DesktopApp> allApps = appReader.ReadDesktopApps(64, "");

  for (const auto &app : allApps)
  {
    list->addRow(new AppRow(list, app));
  }
  // Optionally, you can pass theme to AppRow for per-row theming if needed

>>>>>>> origin/main
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
<<<<<<< HEAD
          return a.name < b.name;
        }
=======
          // Alphabetical tiebreaker
          return a.name < b.name;
        }
        // Zero frequency always at the bottom
>>>>>>> origin/main
        if (fa == 0) return false;
        if (fb == 0) return true;
        return fa > fb;
      });
      for (const auto &app : allApps) {
        list->addRow(new AppRow(list, app));
      }
      return;
    }

<<<<<<< HEAD
=======
    // Only app search: fuzzy match apps
>>>>>>> origin/main
    std::vector<DesktopApp> filteredApps;
    for (const auto &app : appReader.GetAllApps()) {
      if (similarity(search, app.name) > 0.5 || contains(app.name, search, false) != std::string::npos) {
        filteredApps.push_back(app);
      }
    }
<<<<<<< HEAD
=======
    // Sort by frequency (most-run first)
>>>>>>> origin/main
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
<<<<<<< HEAD
        // Sanitize Exec field: remove desktop entry field codes like %f, %F, %u, %U, %i, %c, %k, etc.
        QString rawExec = QString::fromStdString(appRow->app.exec);
        rawExec.replace("%%", "%");
        // remove single-letter field codes preceded by '%'
        QRegularExpression fieldCode("%[fFuUiIcklnNvVmMdD]");
        QString cleanedExec = rawExec;
        cleanedExec.remove(fieldCode);
        auto [program, args, envAssignments] = parseExecCommand(cleanedExec);
=======
        auto [program, args] = parseExecCommand(QString::fromStdString(appRow->app.exec));
>>>>>>> origin/main
        if (program.isEmpty()) return;
        std::cout << "Launching: " << program.toStdString();
        for (const auto &arg : args) std::cout << " " << arg.toStdString();
        std::cout << std::endl;
<<<<<<< HEAD
        appFrequency[appRow->app.exec]++;
        json_util::save_freq(freqPath, appFrequency);
        if (envAssignments.isEmpty()) {
          QProcess::startDetached(program, args);
        } else {
          // Apply env assignments for this process
          QProcess *p = new QProcess();
          QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
          for (const QString &assign : envAssignments) {
            auto pos = assign.indexOf('=');
            if (pos > 0) {
              QString key = assign.left(pos);
              QString val = assign.mid(pos+1);
              env.insert(key, val);
            }
          }
          p->setProcessEnvironment(env);
          p->start(program, args);
          p->setParent(nullptr);
        }
=======
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
>>>>>>> origin/main
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
