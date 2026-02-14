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

  // We'll print a small debug run string after parsing args so its output
  // respects the -v/-vvv flags (verbosity is set while parsing argv).
  // Set default verbosity from env/args later; Debug::setVerbosity() will be
  // called while parsing -v flags.
  AppReader appReader;
  QApplication app(argc, argv);
  ThemeManager themeManager;
  const Theme &theme = themeManager.currentTheme();
  // theme selection may be provided via CLI (--theme <name>); default stylesheet
  // from ~/.config/dlauncher/style.qss will be loaded later if present.
  GlobalEventListener globalKbListener(app);

  // By default include all .desktop entries (don't skip NoDisplay/Hidden/OnlyShowIn)
  bool includeHidden = false;
  bool dumpMode = false;
  bool menuMode = false;
  bool showSystem = false;
  std::string debugAppPattern;
  bool rebuildCache = false;
  bool dumpInMem = false;
  std::string themeName;
  bool testQss = false;
  std::vector<std::pair<std::string, std::string>> menuItems; // label, command
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "-v") {
      Debug::setVerbosity(std::max(Debug::verbosity(), 1));
    } else if (arg == "-vvv") {
      Debug::setVerbosity(std::max(Debug::verbosity(), 3));
    }
    if (arg == "--include-hidden" || arg == "--show-hidden") includeHidden = true;
    else if (arg == "--show-system") showSystem = true;
    else if (arg == "--theme" && i + 1 < argc) { themeName = argv[++i]; }
    else if (arg == "--dump") {
      // Diagnostic mode: print each .desktop path and whether it would be included
      dumpMode = true;
    }
    else if (arg == "--debug-app" && i + 1 < argc) {
      debugAppPattern = argv[++i];
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
    else if (arg == "--rebuild-cache") {
      rebuildCache = true;
    }
    else if (arg == "--test-qss") {
      testQss = true;
    }
    else if (arg == "--dump-inmem") {
      dumpInMem = true;
    }
  }
  // Print a per-run debug identifier now that we've parsed args and set
  // verbosity via -v/-vvv. This respects the user's requested verbosity.
  Debug::log(std::string("Debug run string: ") + Debug::generateRandomString());
  // If rebuildCache is set, disable cache reads so we force a fresh rescan
  appReader.SetCacheEnabled(!rebuildCache);
  appReader.LoadApps(includeHidden, showSystem);
  Debug::log(std::string("[DEBUG] Loaded apps (count) = ") + std::to_string(appReader.GetAllApps().size()));

  // Apply a built-in theme if requested via --theme (or persisted QSS). Track
  // whether a stylesheet is active so we avoid conflicting palette/attributes.
  bool usingQss = false;
  QString appliedQss;

  // If a forced QSS test was requested, use an aggressive global stylesheet to
  // verify whether application-level QSS is honored by the environment.
  if (testQss) {
    usingQss = true;
    appliedQss = QString("QWidget { background: red !important; color: white !important; }");
    Debug::log("[DEBUG] Test QSS will be applied (aggressive red background)");
  }
  if (!themeName.empty()) {
    QString qname = QString::fromStdString(themeName);
    QString qss = themeManager.builtinStyle(qname);
    if (!qss.isEmpty()) {
      usingQss = true;
      appliedQss = qss;
    }
  } else {
    // fallback: load persisted style if present
    QString globalQss = themeManager.styleSheet();
    if (!globalQss.isEmpty()) {
      usingQss = true;
      appliedQss = globalQss;
    }
  }

  // If we decided to use QSS, we will apply it after creating the root
  // container so that both application and root-level selectors are set by
  // ThemeManager::applyStyle. (Avoid applying the QSS here before widgets
  // exist; ThemeManager will set the app stylesheet and palette later.)

  if (dumpMode) {
    // Print diagnostics and exit. If debugAppPattern is set only matching lines are printed.
    appReader.DumpAndPrint(includeHidden, showSystem, debugAppPattern);

    // Also inspect the in-memory app list we loaded and print any matches so we
    // can compare disk scan (DumpAndPrint) vs in-memory filtering.
    if (!debugAppPattern.empty()) {
      std::string pat = toLower(debugAppPattern);
      const auto &all = appReader.GetAllApps();
      for (size_t i = 0; i < all.size(); ++i) {
        const auto &a = all[i];
        std::string line = a.name + "\t" + a.exec;
        std::string low = toLower(line);
        if (low.find(pat) != std::string::npos) {
          std::cout << "[IN-MEM] index=" << i << "\t" << a.name << "\t" << a.exec << "\tNoDisplay=" << (a.noDisplay?"1":"0") << "\tHidden=" << (a.hidden?"1":"0") << std::endl;
        }
      }
    }

    return 0;
  }

  if (rebuildCache) {
    // Persist the freshly scanned apps to cache and exit
    appReader.SaveCache();
    std::cout << "Rebuilt cache (XDG_CACHE_HOME or ~/.cache/dlauncher/apps.cache)." << std::endl;
    return 0;
  }

  if (dumpInMem) {
    const auto &all = appReader.GetAllApps();
    for (size_t i = 0; i < all.size(); ++i) {
      const auto &a = all[i];
      std::cout << "INMEM\t" << i << "\t" << a.name << "\t" << a.exec << "\tNoDisplay=" << (a.noDisplay?"1":"0") << "\tHidden=" << (a.hidden?"1":"0") << std::endl;
    }
    return 0;
  }

  std::string searchTerm = "";

  globalKbListener.registerKeyCallback(Qt::Key_Escape, [&]() {
    app.quit();
  });

  QWidget window;
  window.setObjectName("MainWindow");
  window.setWindowFlags(devFlags());
  window.setFixedSize(theme.windowWidth, theme.windowHeight);
  // If a stylesheet is active, prefer stylesheet painting; don't force
  // translucent background or override the palette which can prevent QSS
  // background rules from taking effect.
  if (usingQss) {
    window.setAttribute(Qt::WA_TranslucentBackground, false);
    window.setAttribute(Qt::WA_StyledBackground, true); // ensure stylesheet paints background
    window.setAutoFillBackground(false);
    if (!appliedQss.isEmpty()) {
      // also apply stylesheet directly to the window to ensure top-level
      // background rules are honored on all platforms
      window.setStyleSheet(appliedQss);
      // Debug info
      qDebug("Applied QSS (len=%d): %s", appliedQss.size(), appliedQss.left(200).toUtf8().constData());
    }
  } else {
    window.setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = window.palette();
    pal.setColor(QPalette::Window, theme.backgroundColor);
    window.setPalette(pal);
    window.setAutoFillBackground(true);
  }

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

  // Create a root container inside the top-level window so stylesheet painting
  // reliably applies to a normal child widget (some platforms don't allow
  // QSS to paint top-level window backgrounds when using translucent flags).
  QWidget *root = new QWidget(&window);
  root->setObjectName("MainWindow");

  ListView *list = new ListView(root);

  QVBoxLayout *layout = new QVBoxLayout(root);

  LockedLineEdit *input = new LockedLineEdit(root, list->listWidget);
  // Input styling is provided by the global QSS generated from ThemeManager;
  // keep the font so widgets without QSS still render consistently.
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
        Debug::log(std::string("Exec: ") + appRow->app.exec);
      }
    } });

  layout->addWidget(list);

  // place the root container inside the window
  root->setLayout(layout);
  root->setGeometry(0, 0, window.width(), window.height());

  // Now apply the theme (global QSS + palette) to the application and root.
  if (usingQss && !appliedQss.isEmpty()) {
    // Write the applied qss to the user's config stylesheet so it's visible
    // in ~/.config/dlauncher/style.qss for inspection.
    themeManager.applyStyle(app, root, appliedQss, themeManager.currentTheme());
    window.setStyleSheet(appliedQss);
  }

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
        {
          std::string launchMsg = std::string("Launching: ") + program.toStdString();
          for (const auto &arg : args) { launchMsg += std::string(" ") + arg.toStdString(); }
          Debug::log(launchMsg);
        }
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

  // If we have a stylesheet to apply, aggressively set it on the window and
  // all child widgets to ensure platforms/compositors that don't honor the
  // application stylesheet still pick up theming.
  if (usingQss && !appliedQss.isEmpty()) {
    std::function<void(QObject*)> applyRec = [&](QObject *o) {
      QWidget *w = qobject_cast<QWidget*>(o);
      if (w) {
        w->setStyleSheet(appliedQss);
      }
      const QObjectList &kids = o->children();
      for (QObject *c : kids) applyRec(c);
    };
    applyRec(&window);
    qDebug("Theme applied recursively to window tree (len=%d)", appliedQss.size());
    // Dump diagnostics so the user can see what was actually applied at runtime
    qDebug() << "APPLICATION styleSheet length=" << qApp->styleSheet().size();
    qDebug() << "WINDOW styleSheet length=" << window.styleSheet().size();
    qDebug() << "APPLICATION palette Window color=" << qApp->palette().color(QPalette::Window).name();
    if (list->listWidget->count() > 0) {
      QListWidgetItem *it = list->listWidget->item(0);
      QWidget *w = list->listWidget->itemWidget(it);
      if (w) {
        qDebug() << "First row objectName=" << w->objectName() << " styleSheetLen=" << w->styleSheet().size();
        qDebug() << "First row styleSheet (prefix):" << w->styleSheet().left(200);
      }
    }
  }

  // Additionally apply a coherent palette and font derived from Theme so that
  // widgets that don't pick up QSS still match the theme.
  if (usingQss) {
    // Use current Theme values as a base
    Theme base = themeManager.currentTheme();
    // if a builtin was requested, override base with it
    if (!themeName.empty()) base = ThemeManager::themeForBuiltin(QString::fromStdString(themeName));
    QPalette pal = app.palette();
    pal.setColor(QPalette::Window, base.backgroundColor);
    pal.setColor(QPalette::WindowText, base.textColor);
    pal.setColor(QPalette::Button, base.rowBackground);
    pal.setColor(QPalette::ButtonText, base.textColor);
    pal.setColor(QPalette::Base, base.inputBackground);
    pal.setColor(QPalette::Text, base.textColor);
    pal.setColor(QPalette::Highlight, base.selectionColor);
    app.setPalette(pal);
    app.setFont(base.font);
    window.setPalette(pal);
    window.setAutoFillBackground(true);
  }

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
