
#include "kb.h"
#include <QApplication>
#include <QDebug>

GlobalEventListener::GlobalEventListener(QApplication &app)
    : QObject(&app) // set app as parent → memory safe
{
  app.installEventFilter(this); // installs itself globally
}

void GlobalEventListener::registerKeyCallback(Qt::Key key,
                                              std::function<void()> callback) {
  keyCallbacks[key].push_back(callback);
}

void GlobalEventListener::registerCtrlNumberCallback(
    int number, std::function<void()> callback) {
  ctrlNumberCallbacks[number].push_back(callback);
}

bool GlobalEventListener::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (!keyEvent)
      return false;

    Qt::Key key = static_cast<Qt::Key>(keyEvent->key());

    if (keyCallbacks.find(key) != keyCallbacks.end()) {
      for (auto &cb : keyCallbacks[key])
        cb();
      return true;
    }

    if (keyEvent->modifiers() & Qt::ControlModifier) {
      int num = keyEvent->key() - Qt::Key_0;
      if (num >= 0 && num <= 9 &&
          ctrlNumberCallbacks.find(num) != ctrlNumberCallbacks.end()) {
        for (auto &cb : ctrlNumberCallbacks[num])
          cb();
        return true;
      }
    }
  }
  return QObject::eventFilter(obj, event);
}
