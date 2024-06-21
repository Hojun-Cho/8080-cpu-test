#include "game.h"
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define GET_HIGH(x) ((x) & 0xff00)
#define GET_LOW(x) ((x) & 0x00ff)
#define SET_HIGH(x, y)                                                         \
  do {                                                                         \
    (x) |= (((uint16_t)y) << 8);                                               \
  } while (0)

#define SET_LOW(x, y)                                                          \
  do {                                                                         \
    (x) |= ((uint16_t)y);                                                      \
  } while (0)

void
error(const char* fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

void
panic(const char* fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static uint8_t
rb(void* udata, uint16_t addr)
{
  Machine* mach;

  mach = udata;
  return mach->mem[addr];
}

static void
wb(void* udata, uint16_t addr, uint8_t val)
{
  Machine* mach;

  mach = udata;
  mach->mem[addr] = val;
}

/*
 *Read
 * 00        INPUTS (Mapped in hardware but never used by the code)
 * 01        INPUTS
 * 02        INPUTS
 * 03        bit shift register read
 *Write
 * 02        shift amount (3 bits)
 * 03        sound bits
 * 04        shift data
 * 05        sound bits
 * 06        watch-dog
 */
static uint8_t
in(void* udata, uint8_t port)
{
  Machine* mach;

  mach = udata;
  switch (port) {
    case 0:
      break;
    case 1:
      return mach->port_1;
    case 2:
      return mach->port_2;
    case 3:
      return (mach->shift >> (8 - mach->shift_offset)) & 0xff;
    default:
      error("unkown port %d", port);
  }
  return 0xff;
}

static void
out(void* udata, uint8_t port, uint8_t val)
{
  Machine* mach;

  mach = udata;
  switch (port) {
    case 2:
      mach->shift_offset = val & 0b111;
      break;
    case 4:
      SET_LOW(mach->shift, GET_HIGH(mach->shift));
      SET_HIGH(mach->shift, val);
      break;
    case 6:
      break;
    default:
      error("not impl not yet");
      break;
  }
}

void
mach_load(Machine* m, const char* fname, uint16_t addr)
{
  SDL_RWops* f;
  Sint64 fsz;

  if ((f = SDL_RWFromFile(fname, "rb")) == 0)
    panic("Can't open file %s\n", fname);
  if ((fsz = SDL_RWsize(f)) > MAX_EACH_ROM)
    panic("Rom file is too large: %d\n", fsz);
  SDL_RWread(f, &m->mem[addr], 1, fsz);
  SDL_RWclose(f);
}

void
mach_init(Machine* m, void (*update)(Machine*))
{
  memset(m, 0, sizeof(Machine));
  cpu_init(&m->cpu, m, rb, wb, in, out);
  m->inter = OP_RST_1;
  m->port_1 = 0;
  m->port_2 = 0;
  m->update_screen = update;
}
