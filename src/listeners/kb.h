#ifndef GLOBALEVENTLISTENER_H
#define GLOBALEVENTLISTENER_H

#include <QKeyEvent>
#include <QObject>
#include <functional>
#include <map>
#include <vector>

class GlobalEventListener : public QObject
{
  Q_OBJECT

public:
  explicit GlobalEventListener(QApplication &app);

  void registerKeyCallback(Qt::Key key, std::function<void()> callback);
  void registerCtrlNumberCallback(int number, std::function<void()> callback);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  std::map<Qt::Key, std::vector<std::function<void()>>> keyCallbacks;
  std::map<int, std::vector<std::function<void()>>> ctrlNumberCallbacks;
};

#endif
