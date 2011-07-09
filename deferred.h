#ifndef DEFERRED_H
#define DEFERRED_H

#include "pendingcallback.h"

#include <QScopedPointer>


class Deferred{
public:
  Deferred();

  void Yield();
  void Callback();

private:
  PendingCallback callback_;
};

#endif // DEFERRED_H
