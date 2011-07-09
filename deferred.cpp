#include "deferred.h"
#include "stackcontext.h"


Deferred::Deferred() {
}

void Deferred::Yield() {
  callback_.Yield();
}

void Deferred::Callback() {
  callback_.ResumeYield();
}
