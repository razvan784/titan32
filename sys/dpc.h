#ifndef _SYS_DPC_H_
#define _SYS_DPC_H_
#include "avrutil.h"

void dpc_post(void (*fn)(int), int param);

#endif
