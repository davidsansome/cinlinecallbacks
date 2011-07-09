  .text
.globl _StackContext_MarkEventLoop
.globl _StackContext_Yield
.globl _StackContext_ResumeYield
.globl _StackContext_ResumeYieldFinished


_StackContext_MarkEventLoop:
  # this->event_loop_sp_ = %rsp + 8;
  lea    0x8(%rsp),%rdx
  mov    %rdx,(%rdi)

  # this->event_loop_bp_ = %rbp;
  mov    %rbp,0x8(%rdi)

  # this->event_loop_ret_ = *%rsp;
  mov    (%rsp),%rdx
  mov    %rdx,0x10(%rdi)

  # return 0;
  mov    $0x0,%eax
  retq
  nop


_StackContext_Yield:
  # Make room for 3 words on the stack:
  # (%rsp)      this
  # 0x8(%rsp)   stack_start
  # 0x10(%rsp)  stack_size
  sub $0x18,%rsp
  mov %rdi,(%rsp)

  # Save the stack between the event loop and the function that called Yield

  # stack_start = rax = %rsp + 0x8 + 0x18
  mov %rsp,%rax
  add $0x20,%rax
  mov %rax,0x8(%rsp)

  # stack_size = rbx = this->event_loop_sp_ - stack_start
  mov (%rdi),%rbx
  sub %rax,%rbx
  mov %rbx,0x10(%rsp)

  # this->yielded_stack_size_ = stack_size
  mov %rbx,0x20(%rdi)

  # this->yielded_stack_data_ = malloc(stack_size)
  mov %rbx,%rdi   # rdi = stack_size
  call malloc     # rax = malloc()

  mov (%rsp),%rbx     # rbx = this
  mov %rax,0x18(%rbx) # rbx->yielded_stack_data_ = rax

  # memcpy(this->yielded_stack_data_, stack_start, stack_size)
  mov 0x18(%rbx),%rdi  # rbx->yielded_stack_data_
  mov 0x8(%rsp),%rsi   # stack_start
  mov 0x10(%rsp),%rdx  # stack_size
  call memcpy

  # Overwrite the saved rip with the address of ResumeYieldFinished, and the
  # saved rbp with the pointer to this context
  mov (%rsp),%rbx     # rbx = this
  mov 0x18(%rbx),%rax # rax = rbx->yielded_stack_data_
  add 0x10(%rsp),%rax # rax += stack_size

  mov $_StackContext_ResumeYieldFinished,%rcx
  mov %rcx,-0x8(%rax)
  mov %rbx,-0x10(%rax)

  # Save the registers and return address from the calling function
  mov (%rsp),%rcx          # rcx = this
  lea 0x20(%rsp),%rbx      # rbx = caller's rsp
  mov %rbx,0x30(%rcx)      # this->yielded_sp_ = stack_start

  mov %rbp,0x38(%rcx)      # this->yielded_bp_ = rbp

  mov 0x18(%rsp),%rbx      # rbx = saved rip
  mov %rbx,0x40(%rcx)      # this->yielded_ret_ = rbx

  # Jump back into the event loop
  mov $0x1,%rax            # return 1
  mov (%rcx),%rsp          # rsp = this->event_loop_sp_
  mov 0x8(%rcx),%rbp       # rbp = this->event_loop_bp_
  jmp *0x10(%rcx)          # jump to this->event_loop_ret_


_StackContext_ResumeYield:
  # Make room for 2 words on the stack:
  # (%rsp)       this
  # 0x8(%rsp)    stack_start
  sub $0x10,%rsp
  mov %rdi,(%rsp)

  # stack_start = this->event_loop_sp_ - this->yielded_stack_size_
  mov (%rdi),%rax     # rax = this->event_loop_sp_
  sub 0x20(%rdi),%rax # rax -= this->yielded_stack_size_
  mov %rax,0x8(%rsp)  # stack_start = rax

  # Backup the stack we're about to replace
  mov 0x20(%rdi),%rdi # rdi = this->yielded_stack_size_
  call malloc         # rax = malloc()
  mov (%rsp),%rbx     # rbx = this
  mov %rax,0x28(%rbx) # this->yielded_stack_backup_ = rax

  mov 0x28(%rbx),%rdi # rdi = this->yielded_stack_backup_
  mov 0x8(%rsp),%rsi  # rsi = stack_start
  mov 0x20(%rbx),%rdx # rdx = this->yielded_stack_size_
  call memcpy

  # Save the registers and return address from the calling function
  mov (%rsp),%rcx          # rcx = this
  lea 0x18(%rsp),%rbx      # rbx = caller's rsp
  mov %rbx,0x48(%rcx)      # this->resuming_sp_ = rbx

  mov %rbp,0x50(%rcx)      # this->resuming_bp_ = rbp

  mov 0x10(%rsp),%rdx      # rdx = saved rip
  mov %rdx,0x58(%rcx)      # this->resuming_ret_ = rdx

  # Load targets into registers before destroying the stack
  mov 0x30(%rcx),%r12      # r12 = this->yielded_sp_
  mov 0x38(%rcx),%r13      # r13 = this->yielded_bp_
  mov 0x40(%rcx),%r14      # r14 = this->yielded_ret_

  # Replace the current stack with the yielded stack
  mov 0x8(%rsp),%rdi       # rdi = stack_start
  mov 0x18(%rcx),%rsi      # rsi = this->yielded_stack_data_
  mov 0x20(%rcx),%rcx      # rcx = this->yielded_stack_size_
  shr $3,%rcx              # rcx /= 8
  rep movsq

  # Jump back into the yielded function
  mov $0x0,%rax
  mov %r12,%rsp
  mov %r13,%rbp
  jmp *%r14


_StackContext_ResumeYieldFinished:
  # -0x10(%rsp)    this
  mov -0x10(%rsp),%rax      # rax = this

  # Load targets into registers before destroying the stack
  mov 0x48(%rax),%r12      # r12 = this->resuming_sp_
  mov 0x50(%rax),%r13      # r13 = this->resuming_bp_
  mov 0x58(%rax),%r14      # r14 = this->resuming_ret_

  # rdi = this->event_loop_sp_ - this->yielded_stack_size
  mov (%rax),%rdi          # rdi = this->event_loop_sp_
  sub 0x20(%rax),%rdi      # rdi -= this->yielded_stack_size_

  # Replace the stack with the resuming stack
  mov 0x18(%rax),%rsi      # rsi = this->yielded_stack_data_
  mov 0x20(%rax),%rcx      # rcx = this->yielded_stack_size_
  shr $3,%rcx              # rcx /= 8
  rep movsq

  # Jump back into the resuming function
  mov $0x0,%rax
  mov %r12,%rsp
  mov %r13,%rbp
  jmp *%r14
