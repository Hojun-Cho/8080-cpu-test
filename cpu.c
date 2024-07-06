#include "cpu.h"
#include <string.h>

#define SET_ZSP(c, val)                                                        \
  do {                                                                         \
    c->zf = (val) == 0;                                                        \
    c->sf = (val) >> 7;                                                        \
    c->pf = parity(val);                                                       \
  } while (0)

enum OPCODES
{
  OP_NOP,
  OP_LXI_B,
  OP_STAX_B,
  OP_INX_B,
  OP_INR_B,
  OP_DCR_B,
  OP_MVI_B,
  OP_RLC,
  OP_INVAL_1,
  OP_DAD_B,
  OP_LDAX_B,
  OP_DCX_B,
  OP_INR_C,
  OP_DCR_C,
  OP_MVI_C,
  OP_RRC,
  OP_INVAL_2,
  OP_LXI_D,
  OP_STAX_D,
  OP_INX_D,
  OP_INR_D,
  OP_DCR_D,
  OP_MVI_D,
  OP_RAL,
  OP_INVAL_3,
  OP_DAD_D,
  OP_LDAX_D,
  OP_DCX_D,
  OP_INR_E,
  OP_DCR_E,
  OP_MVI_E,
  OP_RAR,
  OP_INVAL_4,
  OP_LXI_H,
  OP_SHLD,
  OP_INX_H,
  OP_INR_H,
  OP_DCR_H,
  OP_MVI_H,
  OP_DAA,
  OP_INVAL_5,
  OP_DAD_H,
  OP_LHLD,
  OP_DCX_H,
  OP_INR_L,
  OP_DCR_L,
  OP_MVI_L,
  OP_CMA,
  OP_INVAL_6,
  OP_LXI_SP,
  OP_STA,
  OP_INX_SP,
  OP_INR_M,
  OP_DCR_M,
  OP_MVI_M,
  OP_STC,
  OP_INVAL_7,
  OP_DAD_SP,
  OP_LDA,
  OP_DCX_SP,
  OP_INR_A,
  OP_DCR_A,
  OP_MVI_A,
  OP_CMC,
  OP_MOV_B_B,
  OP_MOV_B_C,
  OP_MOV_B_D,
  OP_MOV_B_E,
  OP_MOV_B_H,
  OP_MOV_B_L,
  OP_MOV_B_M,
  OP_MOV_B_A,
  OP_MOV_C_B,
  OP_MOV_C_C,
  OP_MOV_C_D,
  OP_MOV_C_E,
  OP_MOV_C_H,
  OP_MOV_C_L,
  OP_MOV_C_M,
  OP_MOV_C_A,
  OP_MOV_D_B,
  OP_MOV_D_C,
  OP_MOV_D_D,
  OP_MOV_D_E,
  OP_MOV_D_H,
  OP_MOV_D_L,
  OP_MOV_D_M,
  OP_MOV_D_A,
  OP_MOV_E_B,
  OP_MOV_E_C,
  OP_MOV_E_D,
  OP_MOV_E_E,
  OP_MOV_E_H,
  OP_MOV_E_L,
  OP_MOV_E_M,
  OP_MOV_E_A,
  OP_MOV_H_B,
  OP_MOV_H_C,
  OP_MOV_H_D,
  OP_MOV_H_E,
  OP_MOV_H_H,
  OP_MOV_H_L,
  OP_MOV_H_M,
  OP_MOV_H_A,
  OP_MOV_L_B,
  OP_MOV_L_C,
  OP_MOV_L_D,
  OP_MOV_L_E,
  OP_MOV_L_H,
  OP_MOV_L_L,
  OP_MOV_L_M,
  OP_MOV_L_A,
  OP_MOV_M_B,
  OP_MOV_M_C,
  OP_MOV_M_D,
  OP_MOV_M_E,
  OP_MOV_M_H,
  OP_MOV_M_L,
  OP_HLT,
  OP_MOV_M_A,
  OP_MOV_A_B,
  OP_MOV_A_C,
  OP_MOV_A_D,
  OP_MOV_A_E,
  OP_MOV_A_H,
  OP_MOV_A_L,
  OP_MOV_A_M,
  OP_MOV_A_A,
  OP_ADD_B,
  OP_ADD_C,
  OP_ADD_D,
  OP_ADD_E,
  OP_ADD_H,
  OP_ADD_L,
  OP_ADD_M,
  OP_ADD_A,
  OP_ADC_B,
  OP_ADC_C,
  OP_ADC_D,
  OP_ADC_E,
  OP_ADC_H,
  OP_ADC_L,
  OP_ADC_M,
  OP_ADC_A,
  OP_SUB_B,
  OP_SUB_C,
  OP_SUB_D,
  OP_SUB_E,
  OP_SUB_H,
  OP_SUB_L,
  OP_SUB_M,
  OP_SUB_A,
  OP_SBB_B,
  OP_SBB_C,
  OP_SBB_D,
  OP_SBB_E,
  OP_SBB_H,
  OP_SBB_L,
  OP_SBB_M,
  OP_SBB_A,
  OP_ANA_B,
  OP_ANA_C,
  OP_ANA_D,
  OP_ANA_E,
  OP_ANA_H,
  OP_ANA_L,
  OP_ANA_M,
  OP_ANA_A,
  OP_XRA_B,
  OP_XRA_C,
  OP_XRA_D,
  OP_XRA_E,
  OP_XRA_H,
  OP_XRA_L,
  OP_XRA_M,
  OP_XRA_A,
  OP_ORA_B,
  OP_ORA_C,
  OP_ORA_D,
  OP_ORA_E,
  OP_ORA_H,
  OP_ORA_L,
  OP_ORA_M,
  OP_ORA_A,
  OP_CMP_B,
  OP_CMP_C,
  OP_CMP_D,
  OP_CMP_E,
  OP_CMP_H,
  OP_CMP_L,
  OP_CMP_M,
  OP_CMP_A,
  OP_RNZ,
  OP_POP_B,
  OP_JNZ,
  OP_JMP,
  OP_CNZ,
  OP_PUSH_B,
  OP_ADI,
  OP_RST_0,
  OP_RZ,
  OP_RET,
  OP_JZ,
  OP_INVAL_8,
  OP_CZ,
  OP_CALL,
  OP_ACI,
  OP_RST_1,
  OP_RNC,
  OP_POP_D,
  OP_JNC,
  OP_OUT,
  OP_CNC,
  OP_PUSH_D,
  OP_SUI,
  OP_RST_2,
  OP_RC,
  OP_INVAL_9,
  OP_JC,
  OP_IN,
  OP_CC,
  OP_INVAL_10,
  OP_SBI,
  OP_RST_3,
  OP_RPO,
  OP_POP_H,
  OP_JPO,
  OP_XTHL,
  OP_CPO,
  OP_PUSH_H,
  OP_ANI,
  OP_RST_4,
  OP_RPE,
  OP_PCHL,
  OP_JPE,
  OP_XCHG,
  OP_CPE,
  OP_INVAL_11,
  OP_XRI,
  OP_RST_5,
  OP_RP,
  OP_POP_PSW,
  OP_JP,
  OP_DI,
  OP_CP,
  OP_PUSH_PSW,
  OP_ORI,
  OP_RST_6,
  OP_RM,
  OP_SPHL,
  OP_JM,
  OP_EI,
  OP_CM,
  OP_EXIT,
  OP_CPI,
  OP_RST_7
};

const uint8_t op_cycles[] = {
  4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,
  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 16, 5,  5,  5,
  7,  4,  4,  10, 16, 5,  5,  5,  7,  4,  4,  10, 13, 5,  10, 10, 10, 4,  4,
  10, 13, 5,  5,  5,  7,  4,  5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,
  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,
  5,  5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  7,  7,
  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,  7,  5,  4,  4,  4,  4,  4,
  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,
  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,
  7,  4,  5,  10, 10, 10, 11, 11, 7,  11, 5,  10, 10, 10, 11, 17, 7,  11, 5,
  10, 10, 10, 11, 11, 7,  11, 5,  10, 10, 10, 11, 17, 7,  11, 5,  10, 10, 18,
  11, 11, 7,  11, 5,  5,  10, 4,  11, 17, 7,  11, 5,  10, 10, 4,  11, 11, 7,
  11, 5,  5,  10, 4,  11, 17, 7,  11
};

const char* op_name[] = {
  "NOP",         "LXI  B  ,D16", "STAX B",       "INX  B",       "INR  B",
  "DCR  B",      "MVI  B  ,D8",  "RLC",          "ILL",          "DAD  B",
  "LDAX B",      "DCX  B",       "INR  C",       "DCR  C",       "MVI  C  ,D8",
  "RRC",         "ILL",          "LXI  D  ,D16", "STAX D",       "INX  D",
  "INR  D",      "DCR  D",       "MVI  D  ,D8",  "RAL",          "ILL",
  "DAD  D",      "LDAX D",       "DCX  D",       "INR  E",       "DCR  E",
  "MVI  E  ,D8", "RAR",          "ILL",          "LXI  H  ,D16", "SHLD",
  "INX  H",      "INR  H",       "DCR  H",       "MVI  H  ,D8",  "DAA",
  "ILL",         "DAD  H",       "LHLD",         "DCX  H",       "INR  L",
  "DCR  L",      "MVI  L  ,D8",  "CMA",          "ILL",          "LXI  SP ,D16",
  "STA  A16",    "INX  SP",      "INR  M",       "DCR  M",       "MVI  M  ,D8",
  "STC",         "ILL",          "DAD  SP",      "LDA  A16",     "DCX  SP",
  "INR  A",      "DCR  A",       "MVI  A  ,D8",  "CMC",          "MOV  B  ,B",
  "MOV  B  ,C",  "MOV  B  ,D",   "MOV  B  ,E",   "MOV  B  ,H",   "MOV  B  ,L",
  "MOV  B  ,M",  "MOV  B  ,A",   "MOV  C  ,B",   "MOV  C  ,C",   "MOV  C  ,D",
  "MOV  C  ,E",  "MOV  C  ,H",   "MOV  C  ,L",   "MOV  C  ,M",   "MOV  C  ,A",
  "MOV  D  ,B",  "MOV  D  ,C",   "MOV  D  ,D",   "MOV  D  ,E",   "MOV  D  ,H",
  "MOV  D  ,L",  "MOV  D  ,M",   "MOV  D  ,A",   "MOV  E  ,B",   "MOV  E  ,C",
  "MOV  E  ,D",  "MOV  E  ,E",   "MOV  E  ,H",   "MOV  E  ,L",   "MOV  E  ,M",
  "MOV  E  ,A",  "MOV  H  ,B",   "MOV  H  ,C",   "MOV  H  ,D",   "MOV  H  ,E",
  "MOV  H  ,H",  "MOV  H  ,L",   "MOV  H  ,M",   "MOV  H  ,A",   "MOV  L  ,B",
  "MOV  L  ,C",  "MOV  L  ,D",   "MOV  L  ,E",   "MOV  L  ,H",   "MOV  L  ,L",
  "MOV  L  ,M",  "MOV  L  ,A",   "MOV  M  ,B",   "MOV  M  ,C",   "MOV  M  ,D",
  "MOV  M  ,E",  "MOV  M  ,H",   "MOV  M  ,L",   "HLT",          "MOV  M  ,A",
  "MOV  A  ,B",  "MOV  A  ,C",   "MOV  A  ,D",   "MOV  A  , E",  "MOV  A  ,H",
  "MOV  A  ,L",  "MOV  A  ,M",   "MOV  A  ,A",   "ADD  B",       "ADD  C",
  "ADD  D",      "ADD  E",       "ADD  H",       "ADD  L",       "ADD  M",
  "ADD  A",      "ADC  B",       "ADC  C",       "ADC  D",       "ADC  E",
  "ADC  H",      "ADC  L",       "ADC  M",       "ADC  A",       "SUB  B",
  "SUB  C",      "SUB  D",       "SUB  E",       "SUB  H",       "SUB  L",
  "SUB  M",      "SUB  A",       "SBB  B",       "SBB  C",       "SBB  D",
  "SBB  E",      "SBB  H",       "SBB  L",       "SBB  M",       "SBB  A",
  "ANA  B",      "ANA  C",       "ANA  D",       "ANA  E",       "ANA  H",
  "ANA  L",      "ANA  M",       "ANA  A",       "XRA  B",       "XRA  C",
  "XRA  D",      "XRA  E",       "XRA  H",       "XRA  L",       "XRA  M",
  "XRA  A",      "ORA  B",       "ORA  C",       "ORA  D",       "ORA  E",
  "ORA  H",      "ORA  L",       "ORA  M",       "ORA  A",       "CMP  B",
  "CMP  C",      "CMP  D",       "CMP  E",       "CMP  H",       "CMP  L",
  "CMP  M",      "CMP  A",       "RNZ",          "POP  B",       "JNZ  A16",
  "JMP  A16",    "CNZ  A16",     "PUSH B",       "ADI  D8",      "RST  0",
  "RZ",          "RET",          "JZ   A16",     "ILL",          "CZ   A16",
  "CALL A16",    "ACI  D8",      "RST  1",       "RNC",          "POP  D",
  "JNC  A16",    "OUT  P",       "CNC  A16",     "PUSH D",       "SUI  D8",
  "RST  2",      "RC",           "ILL",          "JC   A16",     "IN   P",
  "CC   A16",    "ILL",          "SBI  D8",      "RST  3",       "RPO",
  "POP  H",      "JPO  A16",     "XTHL",         "CPO  A16",     "PUSH H",
  "ANI  D8",     "RST  4",       "RPE",          "PCHL",         "JPE  A16",
  "XCHG",        "CPE  A16",     "ILL",          "XRI  D8",      "RST  5",
  "RP",          "POP  PSW",     "JP   A16",     "DI",           "CP   A16",
  "PUSH PSW",    "ORI  D8",      "RST  6",       "RM",           "SPHL",
  "JM   A16",    "EI",           "CM   A16",     "ILL",          "CPI  D8",
  "RST  7"
};

static const int op_size[] = {
  1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 3, 1, 1, 1, 1, 2, 1, 1, 1,
  1, 1, 1, 1, 2, 1, 1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 3, 3, 1,
  1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
  1, 1, 3, 2, 3, 1, 2, 1, 1, 1, 3, 2, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1, 1, 1,
  3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1,
};

static uint8_t
rb(CPU* c, uint16_t addr)
{
  return c->rb(c->udata, addr);
}

static void
wb(CPU* c, uint16_t addr, uint8_t val)
{
  c->wb(c->udata, addr, val);
}

static uint16_t
rw(CPU* c, uint16_t addr)
{
  return c->rb(c->udata, addr + 1) << 8 | c->rb(c->udata, addr);
}

static void
ww(CPU* c, uint16_t addr, uint16_t val)
{
  c->wb(c->udata, addr, val & 0xff);
  c->wb(c->udata, addr + 1, val >> 8);
}

static uint8_t
nb(CPU* c)
{
  return rb(c, c->pc++);
}

static uint16_t
nw(CPU* c)
{
  uint16_t res;

  res = rw(c, c->pc);
  c->pc += 2;
  return res;
}

static void
set_bc(CPU* c, uint16_t val)
{
  c->b = val >> 8;
  c->c = val & 0xff;
}

static void
set_de(CPU* c, uint16_t val)
{
  c->d = val >> 8;
  c->e = val & 0xff;
}

static void
set_hl(CPU* c, uint16_t val)
{
  c->h = val >> 8;
  c->l = val & 0xff;
}

static uint16_t
get_bc(CPU* c)
{
  return (c->b << 8) | c->c;
}

static uint16_t
get_de(CPU* c)
{
  return (c->d << 8) | c->e;
}

static uint16_t
get_hl(CPU* c)
{
  return (c->h << 8) | c->l;
}

static void
push_stack(CPU* c, uint16_t val)
{
  c->sp -= 2;
  ww(c, c->sp, val);
}

static uint16_t
pop_stack(CPU* c)
{
  uint16_t val;

  val = rw(c, c->sp);
  c->sp += 2;
  return val;
}

static bool
parity(uint8_t val)
{
  uint8_t n;

  n = 0;
  for (int i = 0; i < 8; ++i) {
    if ((val >> i) & 1)
      ++n;
  }
  if (n & 1)
    return false;
  return true;
}

static bool
carry(int n, uint8_t a, uint8_t b, bool c)
{
  int16_t res, carry;
  int dst;

  dst = 1 << n;
  res = a + b + c;
  carry = res ^ a ^ b;
  if (dst & carry)
    return true;
  return false;
}

static void
add(CPU* c, uint8_t* reg, uint8_t val, bool cy)
{
  uint8_t res;

  res = *reg + val + cy;
  c->cf = carry(8, *reg, val, cy);
  c->hf = carry(4, *reg, val, cy);
  SET_ZSP(c, res);
  *reg = res;
}

static void
sub(CPU* c, uint8_t* reg, uint8_t val, bool cy)
{
  add(c, reg, ~val, !cy);
  c->cf = !c->cf;
}

/* add a word to HL */
static void
dad(CPU* c, uint16_t val)
{
  uint16_t hl;

  hl = get_hl(c);
  c->cf = ((hl + val) >> 16) & 1;
  set_hl(c, hl + val);
}

static uint8_t
inr(CPU* c, uint8_t val)
{
  uint8_t res;

  res = val + 1;
  c->hf = (res & 0xf) == 0;
  SET_ZSP(c, res);
  return res;
}

static uint8_t
dcr(CPU* c, uint8_t val)
{
  uint8_t res;

  res = val - 1;
  c->hf = !((res & 0xf) == 0xf);
  SET_ZSP(c, res);
  return res;
}

static void
ana(CPU* c, uint8_t val)
{
  uint8_t res;

  res = c->a & val;
  c->cf = 0;
  c->hf = ((c->a | val) & 0x08) != 0;
  SET_ZSP(c, res);
  c->a = res;
}

/* xor and store */
static void
xra(CPU* c, uint8_t val)
{
  c->a ^= val;
  c->cf = 0;
  c->hf = 0;
  SET_ZSP(c, c->a);
}

static void
ora(CPU* c, uint8_t val)
{
  c->a |= val;
  c->cf = 0;
  c->hf = 0;
  SET_ZSP(c, c->a);
}

static void
cmp(CPU* c, uint8_t val)
{
  int16_t res;

  res = c->a - val;
  c->cf = res >> 8; /* check carry */
  c->hf = ~(c->a ^ res ^ val) & 0x10;
  SET_ZSP(c, res & 0xff);
}

static void
jmp(CPU* c, uint16_t addr)
{
  c->pc = addr;
}

static void
cond_jmp(CPU* c, bool cond)
{
  uint16_t addr;

  addr = nw(c);
  if (cond)
    c->pc = addr;
}

static void
call(CPU* c, uint16_t addr)
{
  push_stack(c, c->pc);
  jmp(c, addr);
}

static void
cond_call(CPU* c, bool cond)
{
  uint16_t addr;

  addr = nw(c);
  if (cond) {
    call(c, addr);
    c->cyc += 6;
  }
}

static void
ret(CPU* c)
{
  c->pc = pop_stack(c);
}

static void
cond_ret(CPU* c, bool cond)
{
  if (cond) {
    ret(c);
    c->cyc += 6;
  }
}

static void
push_psw(CPU* c)
{
  uint8_t psw;

  psw = 0;
  psw |= c->sf << 7;
  psw |= c->zf << 6;
  psw |= c->hf << 4;
  psw |= c->pf << 2;
  psw |= 1 << 1;
  psw |= c->cf << 0;
  push_stack(c, c->a << 8 | psw);
}

static void
pop_psw(CPU* c)
{
  uint16_t af;
  uint8_t psw;

  af = pop_stack(c);
  c->a = af >> 8;
  psw = af & 0xff;
  c->sf = (psw >> 7) & 1;
  c->zf = (psw >> 6) & 1;
  c->hf = (psw >> 4) & 1;
  c->pf = (psw >> 2) & 1;
  c->cf = (psw >> 0) & 1;
}

/* rotate a left */
static void
rlc(CPU* c)
{
  c->cf = c->a >> 7;
  c->a = (c->a << 1) | c->cf;
}

/* rotate right */
static void
rrc(CPU* c)
{
  c->cf = c->a & 1;
  c->a = (c->a >> 1) | (c->cf << 7);
}

/* rotate left with carry */
static void
ral(CPU* c)
{
  bool cy;

  cy = c->cf;
  c->cf = c->a >> 7;
  c->a = (c->a << 1) | cy;
}

static void
rar(CPU* c)
{
  bool cy;

  cy = c->cf;
  c->cf = c->a & 1;
  c->a = (c->a >> 1) | (cy << 7);
}

static void
daa(CPU* c)
{
  bool cy;
  uint8_t correction;
  uint8_t lsb, msb;

  cy = c->cf;
  correction = 0;
  lsb = c->a & 0x0f;
  msb = c->a >> 4;
  if (c->hf || lsb > 9)
    correction = 0x6;
  if (c->cf || msb > 9 || (msb >= 9 && lsb > 9)) {
    correction += 0x60;
    cy = 1;
  }
  add(c, &c->a, correction, 0);
  c->cf = cy;
}

/* switch de and hl */
static void
xchg(CPU* c)
{
  uint16_t de;

  de = get_de(c);
  set_de(c, get_hl(c));
  set_hl(c, de);
}

static void
xthl(CPU* c)
{
  uint16_t val;

  val = rw(c, c->sp);
  ww(c, c->sp, get_hl(c));
  set_hl(c, val);
}

static void
debug_output(CPU* c, bool diss)
{
  uint8_t f;

  f = 0;
  f |= c->sf << 7;
  f |= c->zf << 6;
  f |= c->hf << 4;
  f |= c->pf << 2;
  f |= 1 << 1;
  f |= c->cf << 0;
  printf("PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, CYC: %lu",
         c->pc,
         c->a << 8 | f,
         get_bc(c),
         get_de(c),
         get_hl(c),
         c->sp,
         c->cyc);
  printf("\t(%02X %02X %02X %02X)",
         rb(c, c->pc),
         rb(c, c->pc + 1),
         rb(c, c->pc + 2),
         rb(c, c->pc + 3));
  if (diss) {
    printf(" - %s", op_name[rb(c, c->pc)]);
  }
  printf("\n");
}

int
cpu_execute(CPU* c, uint8_t opcode)
{
  c->cyc += op_cycles[opcode];

  if (c->inter_delay > 0)
    c->inter_delay -= 1;

  switch (opcode) {
    case OP_MOV_A_A:
      c->a = c->a;
      break;
    case OP_MOV_A_B:
      c->a = c->b;
      break;
    case OP_MOV_A_C:
      c->a = c->c;
      break;
    case OP_MOV_A_D:
      c->a = c->d;
      break;
    case OP_MOV_A_E:
      c->a = c->e;
      break;
    case OP_MOV_A_H:
      c->a = c->h;
      break;
    case OP_MOV_A_L:
      c->a = c->l;
      break;
    case OP_MOV_A_M:
      c->a = rb(c, get_hl(c));
      break;

    case OP_LDAX_B:
      c->a = rb(c, get_bc(c));
      break;
    case OP_LDAX_D:
      c->a = rb(c, get_de(c));
      break;
    case OP_LDA:
      c->a = rb(c, nw(c));
      break;

    case OP_MOV_B_A:
      c->b = c->a;
      break;
    case OP_MOV_B_B:
      c->b = c->b;
      break;
    case OP_MOV_B_C:
      c->b = c->c;
      break;
    case OP_MOV_B_D:
      c->b = c->d;
      break;
    case OP_MOV_B_E:
      c->b = c->e;
      break;
    case OP_MOV_B_H:
      c->b = c->h;
      break;
    case OP_MOV_B_L:
      c->b = c->l;
      break;
    case OP_MOV_B_M:
      c->b = rb(c, get_hl(c));
      break;

    case OP_MOV_C_A:
      c->c = c->a;
      break;
    case OP_MOV_C_B:
      c->c = c->b;
      break;
    case OP_MOV_C_C:
      c->c = c->c;
      break;
    case OP_MOV_C_D:
      c->c = c->d;
      break;
    case OP_MOV_C_E:
      c->c = c->e;
      break;
    case OP_MOV_C_H:
      c->c = c->h;
      break;
    case OP_MOV_C_L:
      c->c = c->l;
      break;
    case OP_MOV_C_M:
      c->c = rb(c, get_hl(c));
      break;

    case OP_MOV_D_A:
      c->d = c->a;
      break;
    case OP_MOV_D_B:
      c->d = c->b;
      break;
    case OP_MOV_D_C:
      c->d = c->c;
      break;
    case OP_MOV_D_D:
      c->d = c->d;
      break;
    case OP_MOV_D_E:
      c->d = c->e;
      break;
    case OP_MOV_D_H:
      c->d = c->h;
      break;
    case OP_MOV_D_L:
      c->d = c->l;
      break;
    case OP_MOV_D_M:
      c->d = rb(c, get_hl(c));
      break;

    case OP_MOV_E_A:
      c->e = c->a;
      break;
    case OP_MOV_E_B:
      c->e = c->b;
      break;
    case OP_MOV_E_C:
      c->e = c->c;
      break;
    case OP_MOV_E_D:
      c->e = c->d;
      break;
    case OP_MOV_E_E:
      c->e = c->e;
      break;
    case OP_MOV_E_H:
      c->e = c->h;
      break;
    case OP_MOV_E_L:
      c->e = c->l;
      break;
    case OP_MOV_E_M:
      c->e = rb(c, get_hl(c));
      break;

    case OP_MOV_H_A:
      c->h = c->a;
      break;
    case OP_MOV_H_B:
      c->h = c->b;
      break;
    case OP_MOV_H_C:
      c->h = c->c;
      break;
    case OP_MOV_H_D:
      c->h = c->d;
      break;
    case OP_MOV_H_E:
      c->h = c->e;
      break;
    case OP_MOV_H_H:
      c->h = c->h;
      break;
    case OP_MOV_H_L:
      c->h = c->l;
      break;
    case OP_MOV_H_M:
      c->h = rb(c, get_hl(c));
      break;

    case OP_MOV_L_A:
      c->l = c->a;
      break;
    case OP_MOV_L_B:
      c->l = c->b;
      break;
    case OP_MOV_L_C:
      c->l = c->c;
      break;
    case OP_MOV_L_D:
      c->l = c->d;
      break;
    case OP_MOV_L_E:
      c->l = c->e;
      break;
    case OP_MOV_L_H:
      c->l = c->h;
      break;
    case OP_MOV_L_L:
      c->l = c->l;
      break;
    case OP_MOV_L_M:
      c->l = rb(c, get_hl(c));
      break;

    case OP_MOV_M_A:
      wb(c, get_hl(c), c->a);
      break;
    case OP_MOV_M_B:
      wb(c, get_hl(c), c->b);
      break;
    case OP_MOV_M_C:
      wb(c, get_hl(c), c->c);
      break;
    case OP_MOV_M_D:
      wb(c, get_hl(c), c->d);
      break;
    case OP_MOV_M_E:
      wb(c, get_hl(c), c->e);
      break;
    case OP_MOV_M_H:
      wb(c, get_hl(c), c->h);
      break;
    case OP_MOV_M_L:
      wb(c, get_hl(c), c->l);
      break;

    case OP_MVI_A:
      c->a = nb(c);
      break;
    case OP_MVI_B:
      c->b = nb(c);
      break;
    case OP_MVI_C:
      c->c = nb(c);
      break;
    case OP_MVI_D:
      c->d = nb(c);
      break;
    case OP_MVI_E:
      c->e = nb(c);
      break;
    case OP_MVI_H:
      c->h = nb(c);
      break;
    case OP_MVI_L:
      c->l = nb(c);
      break;
    case OP_MVI_M:
      wb(c, get_hl(c), nb(c));
      break;

    case OP_STAX_B:
      wb(c, get_bc(c), c->a);
      break;
    case OP_STAX_D:
      wb(c, get_de(c), c->a);
      break;
    case OP_STA: /* word */
      wb(c, nw(c), c->a);
      break;

    case OP_LXI_B:
      set_bc(c, nw(c));
      break;
    case OP_LXI_D:
      set_de(c, nw(c));
      break;
    case OP_LXI_H:
      set_hl(c, nw(c));
      break;
    case OP_LXI_SP:
      c->sp = nw(c);
      break;
    case OP_LHLD:
      set_hl(c, rw(c, nw(c)));
      break;
    case OP_SHLD:
      ww(c, nw(c), get_hl(c));
      break;
    case OP_SPHL:
      c->sp = get_hl(c);
      break;

    case OP_XCHG:
      xchg(c);
      break;
    case OP_XTHL:
      xthl(c);
      break;

    case OP_ADD_A:
      add(c, &c->a, c->a, 0);
      break;
    case OP_ADD_B:
      add(c, &c->a, c->b, 0);
      break;
    case OP_ADD_C:
      add(c, &c->a, c->c, 0);
      break;
    case OP_ADD_D:
      add(c, &c->a, c->d, 0);
      break;
    case OP_ADD_E:
      add(c, &c->a, c->e, 0);
      break;
    case OP_ADD_H:
      add(c, &c->a, c->h, 0);
      break;
    case OP_ADD_L:
      add(c, &c->a, c->l, 0);
      break;
    case OP_ADD_M:
      add(c, &c->a, rb(c, get_hl(c)), 0);
      break;
    case OP_ADI:
      add(c, &c->a, nb(c), 0);
      break;

    case OP_ADC_A:
      add(c, &c->a, c->a, c->cf);
      break;
    case OP_ADC_B:
      add(c, &c->a, c->b, c->cf);
      break;
    case OP_ADC_C:
      add(c, &c->a, c->c, c->cf);
      break;
    case OP_ADC_D:
      add(c, &c->a, c->d, c->cf);
      break;
    case OP_ADC_E:
      add(c, &c->a, c->e, c->cf);
      break;
    case OP_ADC_H:
      add(c, &c->a, c->h, c->cf);
      break;
    case OP_ADC_L:
      add(c, &c->a, c->l, c->cf);
      break;
    case OP_ADC_M:
      add(c, &c->a, rb(c, get_hl(c)), c->cf);
      break;
    case OP_ACI:
      add(c, &c->a, nb(c), c->cf);
      break;

    case OP_SUB_A:
      sub(c, &c->a, c->a, 0);
      break;
    case OP_SUB_B:
      sub(c, &c->a, c->b, 0);
      break;
    case OP_SUB_C:
      sub(c, &c->a, c->c, 0);
      break;
    case OP_SUB_D:
      sub(c, &c->a, c->d, 0);
      break;
    case OP_SUB_E:
      sub(c, &c->a, c->e, 0);
      break;
    case OP_SUB_H:
      sub(c, &c->a, c->h, 0);
      break;
    case OP_SUB_L:
      sub(c, &c->a, c->l, 0);
      break;
    case OP_SUB_M:
      sub(c, &c->a, rb(c, get_hl(c)), 0);
      break;
    case OP_SUI:
      sub(c, &c->a, nb(c), 0);
      break;

    case OP_SBB_A:
      sub(c, &c->a, c->a, c->cf);
      break;
    case OP_SBB_B:
      sub(c, &c->a, c->b, c->cf);
      break;
    case OP_SBB_C:
      sub(c, &c->a, c->c, c->cf);
      break;
    case OP_SBB_D:
      sub(c, &c->a, c->d, c->cf);
      break;
    case OP_SBB_E:
      sub(c, &c->a, c->e, c->cf);
      break;
    case OP_SBB_H:
      sub(c, &c->a, c->h, c->cf);
      break;
    case OP_SBB_L:
      sub(c, &c->a, c->l, c->cf);
      break;
    case OP_SBB_M:
      sub(c, &c->a, rb(c, get_hl(c)), c->cf);
      break;
    case OP_SBI:
      sub(c, &c->a, nb(c), c->cf);
      break;

    case OP_DAD_B:
      dad(c, get_bc(c));
      break;
    case OP_DAD_D:
      dad(c, get_de(c));
      break;
    case OP_DAD_H:
      dad(c, get_hl(c));
      break;
    case OP_DAD_SP:
      dad(c, c->sp);
      break;

    case OP_DI:
      c->iff = 0;
      break;
    case OP_EI:
      c->iff = 1;
      c->inter_delay = 1;
      break;
    case OP_NOP:
      break;
    case OP_HLT:
      c->halted = 1;
      break;

    case OP_INR_A:
      c->a = inr(c, c->a);
      break;
    case OP_INR_B:
      c->b = inr(c, c->b);
      break;
    case OP_INR_C:
      c->c = inr(c, c->c);
      break;
    case OP_INR_D:
      c->d = inr(c, c->d);
      break;
    case OP_INR_E:
      c->e = inr(c, c->e);
      break;
    case OP_INR_H:
      c->h = inr(c, c->h);
      break;
    case OP_INR_L:
      c->l = inr(c, c->l);
      break;
    case OP_INR_M:
      wb(c, get_hl(c), inr(c, rb(c, get_hl(c))));
      break;

    case OP_DCR_A:
      c->a = dcr(c, c->a);
      break;
    case OP_DCR_B:
      c->b = dcr(c, c->b);
      break;
    case OP_DCR_C:
      c->c = dcr(c, c->c);
      break;
    case OP_DCR_D:
      c->d = dcr(c, c->d);
      break;
    case OP_DCR_E:
      c->e = dcr(c, c->e);
      break;
    case OP_DCR_H:
      c->h = dcr(c, c->h);
      break;
    case OP_DCR_L:
      c->l = dcr(c, c->l);
      break;
    case OP_DCR_M:
      wb(c, get_hl(c), dcr(c, rb(c, get_hl(c))));
      break;

    case OP_INX_B:
      set_bc(c, get_bc(c) + 1);
      break;
    case OP_INX_D:
      set_de(c, get_de(c) + 1);
      break;
    case OP_INX_H:
      set_hl(c, get_hl(c) + 1);
      break;
    case OP_INX_SP:
      c->sp += 1;
      break;

    case OP_DCX_B:
      set_bc(c, get_bc(c) - 1);
      break;
    case OP_DCX_D:
      set_de(c, get_de(c) - 1);
      break;
    case OP_DCX_H:
      set_hl(c, get_hl(c) - 1);
      break;
    case OP_DCX_SP:
      c->sp -= 1;
      break;

    case OP_DAA:
      daa(c);
      break;
    case OP_CMA:
      c->a = ~c->a;
      break;
    case OP_STC:
      c->cf = 1;
      break;
    case OP_CMC:
      c->cf = !c->cf;
      break;

    case OP_RLC:
      rlc(c);
      break;
    case OP_RRC:
      rrc(c);
      break;
    case OP_RAL:
      ral(c);
      break;
    case OP_RAR:
      rar(c);
      break;

    case OP_ANA_A:
      ana(c, c->a);
      break;
    case OP_ANA_B:
      ana(c, c->b);
      break;
    case OP_ANA_C:
      ana(c, c->c);
      break;
    case OP_ANA_D:
      ana(c, c->d);
      break;
    case OP_ANA_E:
      ana(c, c->e);
      break;
    case OP_ANA_H:
      ana(c, c->h);
      break;
    case OP_ANA_L:
      ana(c, c->l);
      break;
    case OP_ANA_M:
      ana(c, rb(c, get_hl(c)));
      break;
    case OP_ANI:
      ana(c, nb(c));
      break;

    case OP_XRA_A:
      xra(c, c->a);
      break;
    case OP_XRA_B:
      xra(c, c->b);
      break;
    case OP_XRA_C:
      xra(c, c->c);
      break;
    case OP_XRA_D:
      xra(c, c->d);
      break;
    case OP_XRA_E:
      xra(c, c->e);
      break;
    case OP_XRA_H:
      xra(c, c->h);
      break;
    case OP_XRA_L:
      xra(c, c->l);
      break;
    case OP_XRA_M:
      xra(c, rb(c, get_hl(c)));
      break;
    case OP_XRI:
      xra(c, nb(c));
      break;

    case OP_ORA_A:
      ora(c, c->a);
      break;
    case OP_ORA_B:
      ora(c, c->b);
      break;
    case OP_ORA_C:
      ora(c, c->c);
      break;
    case OP_ORA_D:
      ora(c, c->d);
      break;
    case OP_ORA_E:
      ora(c, c->e);
      break;
    case OP_ORA_H:
      ora(c, c->h);
      break;
    case OP_ORA_L:
      ora(c, c->l);
      break;
    case OP_ORA_M:
      ora(c, rb(c, get_hl(c)));
      break;
    case OP_ORI:
      ora(c, nb(c));
      break;

    case OP_CMP_A:
      cmp(c, c->a);
      break;
    case OP_CMP_B:
      cmp(c, c->b);
      break;
    case OP_CMP_C:
      cmp(c, c->c);
      break;
    case OP_CMP_D:
      cmp(c, c->d);
      break;
    case OP_CMP_E:
      cmp(c, c->e);
      break;
    case OP_CMP_H:
      cmp(c, c->h);
      break;
    case OP_CMP_L:
      cmp(c, c->l);
      break;
    case OP_CMP_M:
      cmp(c, rb(c, get_hl(c)));
      break;
    case OP_CPI:
      cmp(c, nb(c));
      break;

    case OP_JMP:
      jmp(c, nw(c));
      break;
    case OP_JNZ:
      cond_jmp(c, c->zf == 0);
      break;
    case OP_JZ:
      cond_jmp(c, c->zf == 1);
      break;
    case OP_JNC:
      cond_jmp(c, c->cf == 0);
      break;
    case OP_JC:
      cond_jmp(c, c->cf == 1);
      break;
    case OP_JPO:
      cond_jmp(c, c->pf == 0);
      break;
    case OP_JPE:
      cond_jmp(c, c->pf == 1);
      break;
    case OP_JP:
      cond_jmp(c, c->sf == 0);
      break;
    case OP_JM:
      cond_jmp(c, c->sf == 1);
      break;

    case OP_PCHL:
      c->pc = get_hl(c);
      break;
    case OP_CALL:
      call(c, nw(c));
      break;

    case OP_CNZ:
      cond_call(c, c->zf == 0);
      break;
    case OP_CZ:
      cond_call(c, c->zf == 1);
      break;
    case OP_CNC:
      cond_call(c, c->cf == 0);
      break;
    case OP_CC:
      cond_call(c, c->cf == 1);
      break;
    case OP_CPO:
      cond_call(c, c->pf == 0);
      break;
    case OP_CPE:
      cond_call(c, c->pf == 1);
      break;
    case OP_CP:
      cond_call(c, c->sf == 0);
      break;
    case OP_CM:
      cond_call(c, c->sf == 1);
      break;

    case OP_RET:
      ret(c);
      break;
    case OP_RNZ:
      cond_ret(c, c->zf == 0);
      break;
    case OP_RZ:
      cond_ret(c, c->zf == 1);
      break;
    case OP_RNC:
      cond_ret(c, c->cf == 0);
      break;
    case OP_RC:
      cond_ret(c, c->cf == 1);
      break;
    case OP_RPO:
      cond_ret(c, c->pf == 0);
      break;
    case OP_RPE:
      cond_ret(c, c->pf == 1);
      break;
    case OP_RP:
      cond_ret(c, c->sf == 0);
      break;
    case OP_RM:
      cond_ret(c, c->sf == 1);
      break;

    case OP_RST_0:
      call(c, 0x00);
      break;
    case OP_RST_1:
      call(c, 0x08);
      break;
    case OP_RST_2:
      call(c, 010);
      break;
    case OP_RST_3:
      call(c, 0x18);
      break;
    case OP_RST_4:
      call(c, 0x20);
      break;
    case OP_RST_5:
      call(c, 0x28);
      break;
    case OP_RST_6:
      call(c, 0x30);
      break;
    case OP_RST_7:
      call(c, 0x38);
      break;

    case OP_PUSH_B:
      push_stack(c, get_bc(c));
      break;
    case OP_PUSH_D:
      push_stack(c, get_de(c));
      break;
    case OP_PUSH_H:
      push_stack(c, get_hl(c));
      break;
    case OP_PUSH_PSW:
      push_psw(c);
      break;

    case OP_POP_B:
      set_bc(c, pop_stack(c));
      break;
    case OP_POP_D:
      set_de(c, pop_stack(c));
      break;
    case OP_POP_H:
      set_hl(c, pop_stack(c));
      break;
    case OP_POP_PSW:
      pop_psw(c);
      break;

    case OP_IN:
      c->a = c->in(c->udata, nb(c));
      break;
    case OP_OUT:
      c->out(c->udata, nb(c), c->a);
      break;
    default:
      return -1;
  }
  return 0;
}

uint8_t
cpu_fetch(CPU* c)
{
#ifdef DEBUG
  debug_output(c, 1);
#endif
  if (c->inter_pending && c->iff && c->inter_delay == 0) {
    c->inter_pending = 0;
    c->iff = 0;
    c->halted = 0;
    return c->inter_vector;
  } else if (c->halted == false) {
    return nb(c);
  }
  return -1;
}

void
cpu_init(CPU* c, void* udata, void* rb, void* wb, void* in, void* out)
{
  memset(c, 0, sizeof(*c));
  c->udata = udata;
  c->rb = rb;
  c->wb = wb;
  c->in = in;
  c->out = out;
}

void
cpu_inter(CPU* c, uint8_t opcode)
{
  c->inter_pending = 1;
  c->inter_vector = opcode;
}

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
