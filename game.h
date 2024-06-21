#include "cpu.h"

typedef struct Machine Machine;

/*
 *0000-1FFF 8K ROM
 *2000-23FF 1K RAM
 *2400-3FFF 7K Video RAM
 *4000- RAM mirror      => because some An is unconnected
 */
#define MAX_MEM (0x4000)
#define SCREEN_HEIGHT (256)
#define SCREEN_WIDTH (254)

#define MAX_EACH_ROM (0x800)
#define VIDEO_ADDR 0x2400

void
mach_init(Machine* m);
void
error(const char* fmt, ...);
void
panic(const char* fmt, ...);
void
mach_load(Machine* m, const char* fname, uint16_t addr);

struct Machine
{
  CPU cpu;
  uint8_t port_1, port_2;
  uint8_t inter;
  uint16_t shift;
  uint8_t shift_offset;
  uint8_t mem[MAX_MEM];
  uint8_t buf[SCREEN_HEIGHT][SCREEN_WIDTH][4];
  void (*update)(Machine*);
};
