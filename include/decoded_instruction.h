#pragma once

#include "stdint.h"

class DecodedInstr {
public:
  uint32_t opcode;
  uint32_t rs1;
  uint32_t rs2;
  uint32_t func3;
  uint32_t rd;
  uint32_t func7;
  uint32_t imm_i_type;
  uint32_t imm_s_type;
  uint32_t imm_b_type;
  uint32_t imm_u_type;
  uint32_t imm_j_type;
};