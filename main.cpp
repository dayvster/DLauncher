
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

Qt::WindowFlags devFlags()
{
  return Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
}

int main(int argc, char *argv[])
{
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
  if (theme.windowPosX >= 0 && theme.windowPosY >= 0) {
    window.move(theme.windowPosX, theme.windowPosY);
  } else {
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

  for (auto &app : allApps)
  {
    list->addRow(new AppRow(list, &app));
  }
  // Optionally, you can pass theme to AppRow for per-row theming if needed

  layout->addWidget(list);

  QObject::connect(input, &QLineEdit::textChanged, [&](const QString &text)
                   {
    std::string search = text.toStdString();
    list->listWidget->clear();
    if (search.empty()) {
      std::vector<DesktopApp> allApps = appReader.ReadDesktopApps(64, "");
      for (auto &app : allApps) {
        list->addRow(new AppRow(list, &app));
      }
      return;
    }
    std::vector<DesktopApp> filteredApps =
        appReader.SearchApps(search, 64, true);
    for (auto &app : filteredApps) {
      list->addRow(new AppRow(list, &app));
    } });

  QObject::connect(input, &QLineEdit::returnPressed, [&]()
                   {
    int row = list->listWidget->currentRow();
    if (row >= 0 && row < list->listWidget->count()) {
      QListWidgetItem *item = list->listWidget->item(row);
      AppRow *appRow = dynamic_cast<AppRow *>(list->listWidget->itemWidget(item));
      if (appRow) {
        auto [program, args] = parseExecCommand(QString::fromStdString(appRow->app->exec));
        if (program.isEmpty()) return;
        std::cout << "Launching: " << program.toStdString();
        for (const auto &arg : args) std::cout << " " << arg.toStdString();
        std::cout << std::endl;
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
