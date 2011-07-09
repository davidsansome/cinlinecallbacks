#include "stackcontext.h"

#include <QList>
#include <QThreadStorage>


struct StackContext::Private {
  static QThreadStorage<StackContext*> thread_local_;

  QList<PendingCallback> pending_callbacks_;
};

QThreadStorage<StackContext*> StackContext::Private::thread_local_;


StackContext* StackContext::ThreadLocalInstance() {
  if (!Private::thread_local_.hasLocalData()) {
    Private::thread_local_.setLocalData(new StackContext);
  }
  return Private::thread_local_.localData();
}

StackContext::StackContext()
  : d(new Private) {
}

StackContext::~StackContext() {
}

void StackContext::Register(const PendingCallback& callback) {
  d->pending_callbacks_.append(callback);
}

void StackContext::Unregister(const PendingCallback& callback) {
  d->pending_callbacks_.removeAll(callback);
}

void StackContext::ResumeAll() {
  QList<PendingCallback> callbacks(d->pending_callbacks_);
  for (int i=0 ; i<callbacks.count() ; ++i) {
    callbacks[i].ResumeYield();
  }
}
