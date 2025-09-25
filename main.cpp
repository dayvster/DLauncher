
#include "LockedLineEdit.h"
#include "list/list.h"
#include "listeners/kb.h"
#include "src/components/appRow/appRow.h"
#include "src/utils/readApps.h"
#include "utils.h"
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
  window.setFixedSize(500, 300);
  window.setAttribute(Qt::WA_TranslucentBackground);

  ListView *list = new ListView(&window);

  QVBoxLayout *layout = new QVBoxLayout(&window);

  LockedLineEdit *input = new LockedLineEdit(&window, list->listWidget);
  input->setStyleSheet(R"(
        QLineEdit {
            background-color: rgba(0, 0, 0, 120);
            border: 2px solid rgba(0, 0, 0, 150);
            border-radius: 10px;
            padding: 10px;
            color: white;
            font-size: 16px;
        }
    )");
  input->setPlaceholderText("Search...");
  input->setFocus();

  layout->addWidget(input);

  std::vector<DesktopApp> allApps = appReader.ReadDesktopApps(64, "");

  for (auto &app : allApps)
  {
    list->addRow(new AppRow(list, &app));
  }

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
