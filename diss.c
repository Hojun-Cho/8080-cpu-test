#include "cpu.h"
#include "op.h"

void
diss_file(FILE* in, FILE* out, uint16_t pc)
{
  int op;

  while ((op = fgetc(in)) != EOF) {
    fprintf(out, "%10x  %s [ %02x ", pc, op_name[op], op);
    for (int i = 0; i < op_size[op] - 1; ++i)
      fprintf(out, "%02x ", fgetc(in));
    fprintf(out, "]\n");
    pc += op_size[op];
  }
}
