#include "stackcontext.h"

#include <string.h>
#include <stddef.h>

/*static void ResumeYieldFinished() {
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
}*/
