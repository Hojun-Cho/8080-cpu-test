#include "cpu.h"
#include "op.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MEMORY_SIZE 0x10000

uint8_t memory[MEMORY_SIZE];
bool done;

static uint8_t
rb(void* udata, uint16_t addr)
{
  return memory[addr];
}

static void
wb(void* udata, uint16_t addr, uint8_t val)
{
  memory[addr] = val;
}

static uint8_t
in(void* udata, uint8_t port)
{
  return 0x00;
}

static void
out(void* udata, uint8_t port, uint8_t value)
{
  CPU* c;
  uint8_t op;
  uint16_t addr;

  c = (CPU*)udata;
  if (port == 0) {
    done = 1;
  } else if (port == 1) {
    op = c->c;

    if (op == 2) {
      printf("%c", c->e);
    } else if (op == 9) {
      addr = (c->d << 8) | c->e;
      do {
        printf("%c", rb(c, addr++));
      } while (rb(c, addr) != '$');
    }
  }
}

static void
load_file(char* fname, uint16_t addr)
{
  FILE* f;
  size_t fsz;
  size_t rc;

  if ((f = fopen(fname, "rb")) == 0) {
    perror("fopen");
    exit(1);
  }

  fseek(f, 0, SEEK_END);
  fsz = ftell(f);
  rewind(f);
  if (fsz + addr >= MEMORY_SIZE) {
    fprintf(stderr, "%s too large\n", fname);
    exit(1);
  }

  if ((rc = fread(&memory[addr], sizeof(uint8_t), fsz, f)) != fsz) {
    perror("fname");
    exit(1);
  }
  fclose(f);
}

static void
run_test(char* fname, unsigned long exp)
{
  long n_ins;
  CPU c;
  uint8_t opcode;

  printf("*** TEST: %s\n", fname);
  cpu_init(&c, &c, rb, wb, in, out);
  memset(memory, 0, MEMORY_SIZE);
  load_file(fname, 0x100);
  n_ins = 0;
  done = 0;
  c.pc = 0x100;
  memory[0x0000] = 0xD3;
  memory[0x0001] = 0x00;
  memory[0x0005] = 0xD3;
  memory[0x0006] = 0x01;
  memory[0x0007] = 0xC9;
  while (!done) {
    n_ins += 1;
    opcode = cpu_fetch(&c);
    cpu_execute(&c, opcode);
  }
  printf("\n*** %lu instructions executed on %lu cycles"
         " (expected=%lu, diff=%lld)\n\n",
         n_ins,
         c.cyc,
         exp,
         exp - c.cyc);
}

int
main(void)
{
  /*FILE *f = fopen("cpu_tests/TST8080.COM", "r");*/
  /*diss_file(f, stdout, 0x100);*/
  run_test("cpu_tests/TST8080.COM", 4924LU);
  run_test("cpu_tests/CPUTEST.COM", 255653383LU);
  run_test("cpu_tests/8080PRE.COM", 7817LU);
  run_test("cpu_tests/8080EXM.COM", 23803381171LU);
  return 0;
}
