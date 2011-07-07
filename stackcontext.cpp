/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stackcontext.h"

#include <stdio.h>
#include <cstring>

#include <QtDebug>


int StackContext::MarkEventLoop() {
  // Assume x86_64 calling convention where the "this" pointer is passed in
  // rax and nothing is pushed to the stack except rip and rbp.
  void** bp;
  asm("mov %%rbp, %0;" : "=m"(bp));

  event_loop_sp_ = bp + 2;      // +2 for rip and rbp
  event_loop_bp_ = *bp;         // rbp
  event_loop_ret_ = *(bp + 1);  // rip

  qDebug() << "marking event loop sp =" << event_loop_sp_ << "bp =" << event_loop_bp_;

  return 0;
}

void StackContext::Yield() {
  // Assume x86_64 calling convention where the "this" pointer is passed in
  // rax and nothing is pushed to the stack except rip and rbp.

  // We copy the stack beginning with sp + 2 (to remove the rip and rbp that
  // were pushed to call Yield) and ending with the byte before the saved
  // event_loop_sp_.

  void** bp;
  asm("mov %%rbp, %0;" : "=m"(bp));

  const void* stack_start = reinterpret_cast<void*>(bp + 2);
  const void* stack_end   = reinterpret_cast<void*>(event_loop_sp_);
  yielded_stack_size_ = ptrdiff_t(stack_end) - ptrdiff_t(stack_start);

  qDebug() << "saving stack from" << stack_start << "to" << stack_end;

  yielded_stack_data_ = new char[yielded_stack_size_];
  memcpy(yielded_stack_data_, stack_start, yielded_stack_size_);

  // Now save the registers from and return address into the calling function.
  yielded_sp_ = bp + 2;      // +2 for rip and rbp
  yielded_bp_ = *bp;         // rbp
  yielded_ret_ = *(bp + 1);  // rip

  qDebug() << "yielding sp =" << yielded_sp_ << "bp =" << yielded_bp_;

  asm(
    "mov $0x1, %%rax;" // Set return value
    "mov %0, %%rsp;"   // Restore SP
    "mov %1, %%rbp;"   // Restore BP
    "jmp *%2;"         // Jump back to return address
      : // No output registers
      : "m"(event_loop_sp_),
        "m"(event_loop_bp_),
        "m"(event_loop_ret_)
      : "%eax"
  );
}

static void ResumeYieldFinished() {
  qDebug() << "Resume finished";

  StackContext* self;
  asm("mov (%%rbp), %0" : "=r"(self));

  register void* new_sp = self->resuming_sp_;
  register void* new_bp = self->resuming_bp_;
  register void* new_ip = self->resuming_ret_;

  register void* stack_start = reinterpret_cast<void*>(
        ptrdiff_t(self->event_loop_sp_) - self->yielded_stack_size_);
  register void* stack_backup = self->yielded_stack_backup_;
  register size_t stack_size = self->yielded_stack_size_;

  // Replace the stack with the resuming stack.
  memcpy(stack_start, stack_backup, stack_size);

  // Jump back into the resuming function
  asm(
    "mov $0x0, %%rax;"   // Set return value
    "mov %0, %%rsp;"     // Restore SP
    "mov %1, %%rbp;"     // Restore BP
    "jmp *%2;"           // Jump back to return address
      : // No output registers
      : "r"(new_sp),
        "r"(new_bp),
        "r"(new_ip)
      : "%rsp", "%rax"
  );
}

void StackContext::ResumeYield() {
  register void** bp;
  asm("mov %%rbp, %0;" : "=r"(bp));

  register void* stack_start = reinterpret_cast<void*>(
        ptrdiff_t(event_loop_sp_) - yielded_stack_size_);

  qDebug() << "saving stack from" << stack_start;

  // Backup the stack that we're about to replace
  yielded_stack_backup_ = new char[yielded_stack_size_];
  memcpy(yielded_stack_backup_, stack_start, yielded_stack_size_);

  // Overwrite the saved rip with the address of ResumeYieldFinished
  reinterpret_cast<void**>(yielded_stack_data_)[
      yielded_stack_size_ / sizeof(void*) - 1] =
      reinterpret_cast<void*>(ResumeYieldFinished);

  // Overwrite the saved rbp with the pointer to this context
  reinterpret_cast<void**>(yielded_stack_data_)[
      yielded_stack_size_ / sizeof(void*) - 2] =
      this;

  // Save the registers of the calling function
  resuming_sp_ = bp + 2;      // +2 for rip and rbp
  resuming_bp_ = *bp;         // rbp
  resuming_ret_ = *(bp + 1);  // rip

  qDebug() << "restoring stack from" << stack_start;

  register void* new_sp = yielded_sp_;
  register void* new_bp = yielded_bp_;
  register void* new_ip = yielded_ret_;

  // Replace the stack with the yielded stack.
  memcpy(stack_start, yielded_stack_data_, yielded_stack_size_);

  // Jump back into the yielded function
  asm(
    "mov $0x0, %%rax;"   // Set return value
    "mov %0, %%rsp;"     // Restore SP
    "mov %1, %%rbp;"     // Restore BP
    "jmp *%2;"           // Jump back to return address
      : // No output registers
      : "r"(new_sp),
        "r"(new_bp),
        "r"(new_ip)
      : "%rsp", "%rax"
  );
}
