#ifndef _HELLOOS_SYSCALL_H
#define _HELLOOS_SYSCALL_H

#include "task.h"

unsigned long system_call_function(struct pt_regs *regs);

#endif