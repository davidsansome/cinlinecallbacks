#include "pendingcallback.h"
#include "stackcontext.h"

#include <cstdlib>


struct PendingCallback::Private : QSharedData {
  Private()
    : state_(State_Idle),
      data_(new Data) {
  }

  ~Private() {
    delete data_;
  }

  struct Data {
    Data()
      : yielded_stack_data_(NULL),
        yielded_stack_backup_(NULL) {
    }

    ~Data() {
      free(yielded_stack_data_);
      free(yielded_stack_backup_);
    }

    void Yield() asm("_PendingCallback_Yield");
    void ResumeYield() asm("_PendingCallback_ResumeYield");

    StackContext* context_;                   // 0x00

    // Stack to restore when jumping back into the yielded function.
    char* yielded_stack_data_;                // 0x08
    unsigned long long yielded_stack_size_;   // 0x10

    char* yielded_stack_backup_;              // 0x18

    // Registers to restore when jumping back into the yielded function.
    void* yielded_sp_;                        // 0x20
    void* yielded_bp_;                        // 0x28
    void* yielded_ret_;                       // 0x30

    // Registers to restore after ResumeYield.
    void* resuming_sp_;                       // 0x38
    void* resuming_bp_;                       // 0x40
    void* resuming_ret_;                      // 0x48
  };

  State state_;
  Data* data_;
};


PendingCallback::PendingCallback()
  : d(new Private) {
  d->data_->context_ = StackContext::ThreadLocalInstance();
}

PendingCallback::~PendingCallback() {
}

PendingCallback::PendingCallback(const PendingCallback& other)
  : d(other.d) {
}

PendingCallback& PendingCallback::operator =(const PendingCallback& other) {
  d = other.d;
  return *this;
}

bool PendingCallback::operator ==(const PendingCallback& other) const {
  return d.data() == other.d.data();
}

void PendingCallback::Yield() {
  Q_ASSERT(d->state_ == State_Idle);

  StackContext::ThreadLocalInstance()->Register(*this);

  d->state_ = State_Pending;
  d->data_->Yield();
  d->state_ = State_Running;
}

void PendingCallback::ResumeYield() {
  Q_ASSERT(d->state_ == State_Pending);

  StackContext::ThreadLocalInstance()->Unregister(*this);

  d->data_->ResumeYield();
  d->state_ = State_Finished;
}

PendingCallback::State PendingCallback::state() const {
  return d->state_;
}
