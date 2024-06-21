#include "cpu.h"

typedef struct Machine Machine;

/*
 *0000-1FFF 8K ROM
 *2000-23FF 1K RAM
 *2400-3FFF 7K Video RAM
 *4000- RAM mirror      => because some An is unconnected
 */
#define MAX_MEM (0x4000)
void
mach_init(Machine* m);
void
error(const char* fmt, ...);
void
panic(const char* fmt, ...);

struct Machine
{
  CPU cpu;
  uint8_t port_1, port_2;
  uint8_t inter;
  uint16_t shift;
  uint8_t shift_offset;
  uint8_t mem[MAX_MEM];
};
