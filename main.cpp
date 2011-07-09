#include <stdio.h>
#include <cstring>
#include <cstddef>

#include <QtDebug>

#include "deferred.h"
#include "stackcontext.h"

void begin() {
  int var = 42;

  qDebug() << "begin before yield" << var;
  Deferred deferred;
  deferred.Yield();
  qDebug() << "begin after yield" << var;
}

void prebegin() {
  begin();
}

int main(int, char**) {
  StackContext* context = StackContext::ThreadLocalInstance();

  if (context->MarkEventLoop() == 0) {
    qDebug() << "main before begin";
    prebegin();
    qDebug() << "main after begin";
    return 0;
  } else {
    qDebug() << "main yielded";
  }

  context->ResumeAll();
  qDebug() << "main after resume";

  return 0;
}
