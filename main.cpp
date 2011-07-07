#include <stdio.h>
#include <cstring>
#include <cstddef>

#include <QtDebug>

#include "stackcontext.h"

StackContext* sContext = NULL;


void begin() {
  int var = 42;

  qDebug() << "begin before yield" << var;
  sContext->Yield();
  qDebug() << "begin after yield" << var;
}

void prebegin() {
  begin();
}

int main(int, char**) {
  sContext = new StackContext;

  if (sContext->MarkEventLoop() == 0) {
    qDebug() << "main before begin";
    prebegin();
    qDebug() << "main after begin";
    return 0;
  } else {
    qDebug() << "main yielded";
  }

  sContext->ResumeYield();
  qDebug() << "main after resume";

  return 0;
}
