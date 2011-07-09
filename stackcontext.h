#ifndef STACKCONTEXT_H
#define STACKCONTEXT_H


class StackContext {
public:
  int MarkEventLoop() asm("_StackContext_MarkEventLoop");
  void Yield() asm("_StackContext_Yield");
  void ResumeYield() asm("_StackContext_ResumeYield");

private:
  // Registers to restore when jumping back to the event loop.  sp is also the
  // start of (byte above) the stack data that is saved when yielding.
  void** event_loop_sp_;                    // 0x00
  void* event_loop_bp_;                     // 0x08
  void* event_loop_ret_;                    // 0x10

  // Stack to restore when jumping back into the yielded function.
  char* yielded_stack_data_;                // 0x18
  unsigned long long yielded_stack_size_;   // 0x20

  char* yielded_stack_backup_;              // 0x28

  // Registers to restore when jumping back into the yielded function.
  void** yielded_sp_;                       // 0x30
  void* yielded_bp_;                        // 0x38
  void* yielded_ret_;                       // 0x40

  // Registers to restore after ResumeYield.
  void** resuming_sp_;                      // 0x48
  void* resuming_bp_;                       // 0x50
  void* resuming_ret_;                      // 0x58
};

#endif // STACKCONTEXT_H
