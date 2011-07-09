#ifndef STACKCONTEXT_H
#define STACKCONTEXT_H

#include "pendingcallback.h"

#include <QScopedPointer>


class StackContext {
public:
  static StackContext* ThreadLocalInstance();
  ~StackContext();

  int MarkEventLoop() asm("_StackContext_MarkEventLoop");

  void ResumeAll();

private:
  StackContext();

private:
  friend class PendingCallback;
  void Register(const PendingCallback& callback);
  void Unregister(const PendingCallback& callback);

private:
  Q_DISABLE_COPY(StackContext);

  // Registers to restore when jumping back to the event loop.  sp is also the
  // start of (byte above) the stack data that is saved when yielding.
  void* event_loop_sp_;                     // 0x00
  void* event_loop_bp_;                     // 0x08
  void* event_loop_ret_;                    // 0x10

  struct Private;
  QScopedPointer<Private> d;
};

#endif // STACKCONTEXT_H
