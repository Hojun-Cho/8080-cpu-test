#ifndef _CPU_
#define _CPU_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct CPU CPU;

void
cpu_init(CPU* c, void* udata, void* rb, void* wb, void* in, void* out);
uint8_t
cpu_fetch(CPU* c);
int
cpu_execute(CPU* c, uint8_t opcode);
void
cpu_inter(CPU* c, uint8_t opcode);
void
diss_file(FILE* in, FILE* out, uint16_t pc);

struct CPU
{
  uint8_t (*rb)(void*, uint16_t);
  void (*wb)(void*, uint16_t, uint8_t);
  uint8_t (*in)(void*, uint8_t);
  void (*out)(void*, uint8_t, uint8_t);

  void* udata;
  unsigned long cyc;

  uint16_t pc, sp;
  uint8_t a, b, c, d, e, h, l;
  bool sf, zf, hf, pf, cf, iff;
  bool halted;
  bool inter_pending;
  uint8_t inter_vector;
  uint8_t inter_delay;
};

#endif
