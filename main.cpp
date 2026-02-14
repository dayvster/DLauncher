#include <fstream>
#include <filesystem>

#include "LockedLineEdit.h"
#include "ui/components/list.h"
#include "core/listeners/kb.h"
#include "ui/components/appRow.h"
#include "core/apps/readApps.h"
#include "utils.h"
#include "theme/theme.h"
#include "utils/utils.h"
#include "utils/debug.h"
#include "utils/json.hpp"
#include "core/xdg.h"
#include "core/frequency_store.h"
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
#include <QRegularExpression>

Qt::WindowFlags devFlags()
{
  return Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
}

int main(int argc, char *argv[])
{
  // Use XDG helpers and FrequencyStore
  std::string freqPath = xdg::configPath("dlauncher_freq.txt").string();
  // ensure dir exists
  xdg::ensureDir(xdg::configPath(""));
  // frequency store
  FrequencyStore freqStore(freqPath);

  Debug::log("Debug run string: " + Debug::generateRandomString());
  AppReader appReader;
  QApplication app(argc, argv);
  ThemeManager themeManager;
  const Theme &theme = themeManager.currentTheme();
  GlobalEventListener globalKbListener(app);

  // By default include all .desktop entries (don't skip NoDisplay/Hidden/OnlyShowIn)
  bool includeHidden = false;
  bool dumpMode = false;
  bool menuMode = false;
  bool showSystem = false;
  std::vector<std::pair<std::string, std::string>> menuItems; // label, command
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "--include-hidden" || arg == "--show-hidden") includeHidden = true;
    else if (arg == "--show-system") showSystem = true;
    else if (arg == "--dump") {
      // Diagnostic mode: print each .desktop path and whether it would be included
      dumpMode = true;
    }
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
  appReader.LoadApps(includeHidden, showSystem);

  if (dumpMode) {
    // Print diagnostics and exit
    appReader.DumpAndPrint(includeHidden, showSystem);
    return 0;
  }

  std::string searchTerm = "";

  globalKbListener.registerKeyCallback(Qt::Key_Escape, [&]() {
    app.quit();
  });

  QWidget window;
  window.setWindowFlags(devFlags());
  window.setFixedSize(theme.windowWidth, theme.windowHeight);
  window.setAttribute(Qt::WA_TranslucentBackground);
  QPalette pal = window.palette();
  pal.setColor(QPalette::Window, theme.backgroundColor);
  window.setPalette(pal);
  window.setAutoFillBackground(true);

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

    // Create rows lazily from pointers into the canonical app list
    if (menuMode) {
      for (auto &p : menuItems) {
        DesktopApp a;
        a.name = p.first;
        a.exec = p.second;
        list->addRow(new AppRow(list, a));
      }
    } else {
      auto &master = appReader.GetAllApps();
      size_t count = master.size();
      for (size_t i = 0; i < count && i < 64; ++i) {
        list->addRow(new AppRow(list, master[i]));
      }
    }

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
        int fa = freqStore.get(a.exec);
        int fb = freqStore.get(b.exec);
        if (fa == fb) {
          return a.name < b.name;
        }
        if (fa == 0) return false;
        if (fb == 0) return true;
        return fa > fb;
      });
      for (const auto &app : allApps) {
        list->addRow(new AppRow(list, app));
      }
      return;
    }

  std::vector<const DesktopApp*> filteredAppsPtrs;
    for (const auto &app : appReader.GetAllApps()) {
      if (similarity(search, app.name) > 0.5 || contains(app.name, search, false) != std::string::npos) {
        filteredAppsPtrs.push_back(&app);
      }
    }

    std::sort(filteredAppsPtrs.begin(), filteredAppsPtrs.end(), [&](const DesktopApp *a, const DesktopApp *b) {
      int fa = freqStore.get(a->exec);
      int fb = freqStore.get(b->exec);
      if (fa == fb) {
        return a->name < b->name;
      }
      if (fa == 0) return false;
      if (fb == 0) return true;
      return fa > fb;
    });
    for (const auto *appPtr : filteredAppsPtrs) {
      list->addRow(new AppRow(list, *appPtr));
    } });

  QObject::connect(input, &QLineEdit::returnPressed, [&]()
                   {
    int row = list->listWidget->currentRow();
    if (row >= 0 && row < list->listWidget->count()) {
      QListWidgetItem *item = list->listWidget->item(row);
      AppRow *appRow = dynamic_cast<AppRow *>(list->listWidget->itemWidget(item));
      if (appRow) {
        // Sanitize Exec field: remove desktop entry field codes like %f, %F, %u, %U, %i, %c, %k, etc.
        QString rawExec = QString::fromStdString(appRow->app.exec);
        rawExec.replace("%%", "%");
        // remove single-letter field codes preceded by '%'
        QRegularExpression fieldCode("%[fFuUiIcklnNvVmMdD]");
        QString cleanedExec = rawExec;
        cleanedExec.remove(fieldCode);
        auto [program, args, envAssignments] = parseExecCommand(cleanedExec);
        if (program.isEmpty()) return;
        std::cout << "Launching: " << program.toStdString();
        for (const auto &arg : args) std::cout << " " << arg.toStdString();
        std::cout << std::endl;
        freqStore.inc(appRow->app.exec);
        freqStore.save();
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
        app.quit();
      }
    } });

  QRect screenGeometry = QApplication::primaryScreen()->geometry();
  int x = (screenGeometry.width() - window.width()) / 2;
  int y = (screenGeometry.height() - window.height()) / 2;
  window.move(x, y);

  window.show();

  // Safety: ensure initial rows are populated (some environments/layout timing
  // issues can leave the list empty). If list is empty, populate from master.
  if (!menuMode && list->listWidget->count() == 0) {
    auto &master = appReader.GetAllApps();
    size_t count = master.size();
    for (size_t i = 0; i < count && i < 64; ++i) {
      list->addRow(new AppRow(list, master[i]));
    }
  }

  return app.exec();
}
