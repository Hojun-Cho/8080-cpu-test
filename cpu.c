#include "cpu.h"
#include "op.h"
#include <stdio.h>
#include <string.h>

#define SET_ZSP(c, val)                                                        \
  do {                                                                         \
    c->zf = (val) == 0;                                                        \
    c->sf = (val) >> 7;                                                        \
    c->pf = parity(val);                                                       \
  } while (0)

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

static int
execute(CPU* c, enum OPCODES opcode)
{
  c->cyc += op_cycles[opcode];

  if (c->interrupt_delay > 0)
    c->interrupt_delay -= 1;

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
      c->interrupt_delay = 1;
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

int
cpu_run(CPU* c)
{
  if (c->interrupt_pending && c->iff && c->interrupt_delay == 0) {
    c->interrupt_pending = 0;
    c->iff = 0;
    c->halted = 0;
    return execute(c, c->interrupt_vector);
  } else if (c->halted == false) {
    return execute(c, nb(c));
  }
  return -1;
}

void
cpu_init(CPU* c, void* udata, void* rb, void* wb, void* in, void* out)
{
  memset(c, 0, sizeof(*c));
  c->udata = c;
  c->rb = rb;
  c->wb = wb;
  c->in = in;
  c->out = out;
}

void
interrupt(CPU* c, uint8_t opcode)
{
  c->interrupt_pending = 1;
  c->interrupt_vector = opcode;
}

void
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

#undef SET_ZSP
