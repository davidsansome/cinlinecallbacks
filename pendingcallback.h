#ifndef PENDINGCALLBACK_H
#define PENDINGCALLBACK_H

#include <QExplicitlySharedDataPointer>


class PendingCallback {
public:
  enum State {
    State_Idle,
    State_Pending,
    State_Running,
    State_Finished,
  };

  PendingCallback();
  PendingCallback(const PendingCallback& other);
  ~PendingCallback();

  PendingCallback& operator =(const PendingCallback& other);
  bool operator ==(const PendingCallback& other) const;

  void Yield();
  void ResumeYield();

  State state() const;

private:
  struct Private;
  QExplicitlySharedDataPointer<Private> d;
};

#endif // PENDINGCALLBACK_H
