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

#ifndef STACKCONTEXT_H
#define STACKCONTEXT_H

#include <cstddef>

class StackContext {
public:
  int MarkEventLoop();
  void Yield();
  void ResumeYield();


  // Registers to restore when jumping back to the event loop.  sp is also the
  // start of (byte above) the stack data that is saved when yielding.
  void** event_loop_sp_;
  void* event_loop_bp_;
  void* event_loop_ret_;

  // Stack to restore when jumping back into the yielded function.
  char* yielded_stack_data_;
  size_t yielded_stack_size_;

  char* yielded_stack_backup_;

  // Registers to restore when jumping back into the yielded function.
  void** yielded_sp_;
  void* yielded_bp_;
  void* yielded_ret_;

  // Registers to restore after ResumeYield.
  void** resuming_sp_;
  void* resuming_bp_;
  void* resuming_ret_;
};

#endif // STACKCONTEXT_H
