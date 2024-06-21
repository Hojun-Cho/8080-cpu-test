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
int
cpu_load(CPU* cpu, const char* fname, uint16_t addr);

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

static const uint8_t op_cycles[] = {
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

static const char* op_name[] = {
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
#endif
