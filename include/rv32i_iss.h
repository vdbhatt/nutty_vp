#pragma once

#include "constants.h"
#include "decoded_instruction.h"
#include "simple_memory_interface.h"
#include "stdint.h"
#include <vector>
extern bool debug;
class RV32I_ISS {
public:
  uint32_t regs[32];
  uint32_t csr[4096];
  uint32_t pc;
  uint32_t next_pc;
  MemoryInterface *_mem_interface;

  ~RV32I_ISS() {}
  RV32I_ISS(MemoryInterface *mem_interface)
      : _mem_interface{mem_interface}, pc{0}, next_pc{0} {}
  uint32_t fetch() { return (*_mem_interface).load_mem(pc, 4); }

  int32_t sign_extend(uint32_t n, int bitsize) {
    uint32_t all_ones = 0xffffffff;
    uint32_t mask = all_ones >> (32 - bitsize);
    return n & (1 << (bitsize - 1)) ? ~mask | n : n;
  }
  DecodedInstr *get_op_type(uint32_t instr) {
    DecodedInstr *decoded_inst = new DecodedInstr();

    decoded_inst->opcode = instr & (0x7F);
    decoded_inst->rd = (instr & (0x1F << 7)) >> 7;
    decoded_inst->func3 = (instr & (0x7 << 12)) >> 12;
    decoded_inst->rs1 = (instr & (0x1F << 15)) >> 15;
    decoded_inst->rs2 = (instr & (0x1F << 20)) >> 20;
    decoded_inst->func7 = (instr & (0x7F << 25)) >> 25;
    decoded_inst->imm_i_type = ((instr >> 20) & 0x00000FFF);
    decoded_inst->imm_s_type =
        (((instr & (0x7F << 25)) >> 25) << 5) | ((instr & (0x1F << 7)) >> 7);
    decoded_inst->imm_b_type = (instr & 0x80000000) >> 19 |
                               ((instr & 0x80) << 4) | ((instr >> 20) & 0x7E0) |
                               ((instr >> 7) & 0x1E);
    decoded_inst->imm_u_type = (instr & 0xFFFFF000);
    decoded_inst->imm_j_type = ((instr & 0x80000000) >> 11) |
                               (instr & 0xff000) | ((instr >> 9) & 0x800) |
                               ((instr >> 20) & 0x7FE);
    decoded_inst->imm_i_type = sign_extend(decoded_inst->imm_i_type, 12);
    decoded_inst->imm_s_type = sign_extend(decoded_inst->imm_s_type, 12);
    decoded_inst->imm_b_type = sign_extend(decoded_inst->imm_b_type, 13);
    decoded_inst->imm_j_type = sign_extend(decoded_inst->imm_j_type, 21);
    return decoded_inst;
  }
  void write_reg(int rd, uint32_t rd_data) {
    if (rd != 0) {
      regs[rd] = rd_data;
    }
  }
  uint32_t execute(uint32_t instr) {
    DecodedInstr *decoded = get_op_type(instr);
    uint32_t rd_data = 0;
    switch (decoded->opcode) {
    case 0b0110111: // lui
      rd_data = decoded->imm_u_type;
      write_reg(decoded->rd, rd_data);
      break;
    case 0b0010111: // auipc
      rd_data = decoded->imm_u_type + pc;
      write_reg(decoded->rd, rd_data);
      break;
    case 0b1101111:     // jal
      rd_data = pc + 4; // store then next pc into rd
      write_reg(decoded->rd, rd_data);
      pc = pc + decoded->imm_j_type - 4; // compensate for + 4 after execute
      break;
    case 0b1100111: // jalr
      rd_data = pc + 4;
      pc = (0xfffffffe & (decoded->imm_i_type + regs[decoded->rs1])) - 4;
      write_reg(decoded->rd, rd_data);
      break;
    case 0b1100011: // branch
      switch (decoded->func3) {
      case 0b000: // beq
        if (regs[decoded->rs1] == regs[decoded->rs2])
          pc = decoded->imm_b_type + pc - 4;
        break;
      case 0b001: // bne
        if (regs[decoded->rs1] != regs[decoded->rs2])
          pc = decoded->imm_b_type + pc - 4;
        break;
      case 0b100: // blt
        if ((int32_t)regs[decoded->rs1] < (int32_t)regs[decoded->rs2])
          pc = decoded->imm_b_type + pc - 4;
        break;
      case 0b101: // bge
        if ((int32_t)regs[decoded->rs1] >= (int32_t)regs[decoded->rs2])
          pc = decoded->imm_b_type + pc - 4;
        break;
      case 0b110: // bltu
        if (regs[decoded->rs1] < regs[decoded->rs2])
          pc = decoded->imm_b_type + pc - 4;
        break;
      case 0b111: // bgeu
        if (regs[decoded->rs1] >= regs[decoded->rs2])
          pc = decoded->imm_b_type + pc - 4;
        break;
      default:
        call_exit(decoded, instr);
      }
      break;
    case 0b0000011: // load
    {
      uint32_t addr = regs[decoded->rs1] + decoded->imm_i_type;
      switch (decoded->func3) {
      case 0b000: // LB
        rd_data = load_byte(addr);
        write_reg(decoded->rd, rd_data);
        // ////printf("load addr = %x , data = %x\n", addr, load_byte(addr));
        break;
      case 0b001: // LH
        rd_data = load_half(addr);
        write_reg(decoded->rd, rd_data);
        break;
      case 0b010: // LW
        // if(addr ==0x700)
        // {
        //     //printf("break");
        // }
        rd_data = load_word(addr);
        // printf("load addr = %08x, data = %08x \n", addr, rd_data);
        write_reg(decoded->rd, rd_data);
        break;
      case 0b100: // LBU
        rd_data = load_byte_unsigned(addr);
        write_reg(decoded->rd, rd_data);
        break;
      case 0b101: // LHU
        rd_data = load_half_unsigned(addr);
        write_reg(decoded->rd, rd_data);
        break;
      default:
        call_exit(decoded, instr);
      }
    } break;
    case 0b0100011: // store
    {
      uint32_t addr;
      switch (decoded->func3) {
      case 0b000: // SB
        addr = regs[decoded->rs1] + decoded->imm_s_type;
        // ////printf("store addr = %x, data = %x\n", addr, regs[decoded->rs2]);
        store_byte(addr, regs[decoded->rs2]);
        break;
      case 0b001: // SH
        addr = regs[decoded->rs1] + decoded->imm_s_type;
        ////printf("store addr = %x, data = %x \n", addr, regs[decoded->rs2]);
        store_half(addr, regs[decoded->rs2]);
        break;
      case 0b010: // SW
        addr = regs[decoded->rs1] + decoded->imm_s_type;
        // printf("store addr = %x, data = %x \n", addr, regs[decoded->rs2]);
        store_word(addr, regs[decoded->rs2]);
        break;
      }
      break;
    }
    case 0b0010011: // immediate
      switch (decoded->func3) {
      case 0b000: // addi
        rd_data = (int32_t)decoded->imm_i_type + (int32_t)regs[decoded->rs1];
        write_reg(decoded->rd, rd_data);
        break;
      case 0b010: // slti
        rd_data =
            (int32_t)regs[decoded->rs1] < (int32_t)decoded->imm_i_type ? 1 : 0;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b011: // sltiu
        rd_data = regs[decoded->rs1] < decoded->imm_i_type ? 1 : 0;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b100: // xori
        rd_data = regs[decoded->rs1] ^ decoded->imm_i_type;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b110: // ori
        rd_data = regs[decoded->rs1] | decoded->imm_i_type;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b111: // andi
        rd_data =
            regs[decoded->rs1] & decoded->imm_i_type; // same as decoded->rs2;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b001: // slli
        rd_data = regs[decoded->rs1] << (decoded->imm_i_type & (0x1F));
        write_reg(decoded->rd, rd_data);
        break;
      case 0b101: // srli / srai
        switch (decoded->func7) {
        case 0b0000000: // srli
          // 0x000ffff & shifted value 32-
          rd_data = regs[decoded->rs1] >>
                    (decoded->imm_i_type & (0x1F)); // works for gcc!!
          write_reg(decoded->rd, rd_data);
          break;
          break;
        case 0b0100000: // srai i.e the sign bit is preserved
          rd_data = (int32_t)regs[decoded->rs1] >>
                    (int32_t)(decoded->imm_i_type & (0x1F));
          write_reg(decoded->rd, rd_data);
          break;
          break;
        default:
          call_exit(decoded, instr);
        }
        break;
      default:
        call_exit(decoded, instr);
      }
      break;
    case 0b0110011: // rtype
      switch (decoded->func3) {
      case 0b000: // add / sub
        switch (decoded->func7) {
        case 0b0000000:
          rd_data = (int32_t)regs[decoded->rs1] + (int32_t)regs[decoded->rs2];
          write_reg(decoded->rd, rd_data);
          break;
        case 0b0100000:
          rd_data = (int32_t)regs[decoded->rs1] - (int32_t)regs[decoded->rs2];
          write_reg(decoded->rd, rd_data);
          break;
        default:
          call_exit(decoded, instr);
        }
        break;
      case 0b001: // sll
        rd_data = regs[decoded->rs1] << (regs[decoded->rs2] & (0x1F));
        write_reg(decoded->rd, rd_data);
        break;
      case 0b010: // slt
        rd_data = (int32_t)regs[decoded->rs1] < (int32_t)regs[decoded->rs2]
                      ? 0x1
                      : 0x0;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b011: // sltu
        rd_data = regs[decoded->rs1] < regs[decoded->rs2] ? 0x1 : 0x0;
        write_reg(decoded->rd, rd_data);
        break;
      case 0b100: // xor
        rd_data = regs[decoded->rs1] ^ regs[decoded->rs2];
        write_reg(decoded->rd, rd_data);
        break;
      case 0b101: // srl / sra
        switch (decoded->func7) {
        case 0b0000000: // srl
          rd_data = regs[decoded->rs1] >>
                    (regs[decoded->rs2] & (0x1F)); // works for gcc!!
          write_reg(decoded->rd, rd_data);
          break;
        case 0b0100000: // srai i.e the sign bit is preserved
          rd_data = (int32_t)regs[decoded->rs1] >>
                    (int32_t)(regs[decoded->rs2] & (0x1F));
          write_reg(decoded->rd, rd_data);
          break;
        default:
          call_exit(decoded, instr);
        }
        break;
      case 0b110: // or
        rd_data = regs[decoded->rs1] | regs[decoded->rs2];
        write_reg(decoded->rd, rd_data);
        break;
      case 0b111: // and
        rd_data = regs[decoded->rs1] & regs[decoded->rs2];
        write_reg(decoded->rd, rd_data);
        break;
      default:
        call_exit(decoded, instr);
      }
      break;
    case 0b0001111: // fence
      ////printf("fence!!");
      break;
    case 0b1110011: // system
    {
      uint32_t csr_addr = (0x00000fff & (instr >> 20));
      uint32_t csr_old_value;
      uint32_t csr_uimm = decoded->rs1;

      switch (decoded->func3) {
      case 0b000: // ecall/ebreak/xret
      {

        if (decoded->func7 == 0 && decoded->rs2 == 0) // ecall
        {
          printf("ecall");
          // call_exit(decoded, instr);
          csr[mepc] = pc;
        } else if (decoded->func7 == 0 && decoded->rs2 == 1) // ebreak
        {
          ////printf("ebreak");
          call_exit(decoded, instr);
        } else if (decoded->func7 == 0b0011000 &&
                   decoded->rs2 == 0b00010) // mret
        {
          printf("mret");
          pc = csr[mepc] - 4; // compensate for +4 after execute
        }
      } break;
      case 0b001: // csrrw
        csr_old_value = csr[csr_addr];
        csr[csr_addr] = regs[decoded->rs1];
        write_reg(decoded->rd, csr_old_value);
        break;
      case 0b010: // csrrs
        csr_old_value = csr[csr_addr];
        if (decoded->rs1 != 0)
          csr[csr_addr] = csr_old_value | regs[decoded->rs1];
        write_reg(decoded->rd, csr_old_value);
        break;
      case 0b011: // csrrc
        csr_old_value = csr[csr_addr];
        if (decoded->rs1 != 0)
          csr[csr_addr] = csr_old_value & (~regs[decoded->rs1]);
        write_reg(decoded->rd, csr_old_value);
        break;
      case 0b101: // csrrwi
        csr_old_value = csr[csr_addr];
        csr[csr_addr] = decoded->rs1;
        write_reg(decoded->rd, csr_old_value);
        break;
      case 0b110: // csrrsi
        csr_old_value = csr[csr_addr];
        if (decoded->rs1 != 0)
          csr[csr_addr] = csr_old_value | decoded->rs1;
        write_reg(decoded->rd, csr_old_value);
        break;
      case 0b111: // csrrci
        csr_old_value = csr[csr_addr];
        if (decoded->rs1 != 0)
          csr[csr_addr] = csr_old_value & (~(decoded->rs1));
        write_reg(decoded->rd, csr_old_value);
        break;
      default:
        ////printf("csr error ");
        call_exit(decoded, instr);
        break;
      }
      break;
    }
    default:
      call_exit(decoded, instr);
      break;
    }
    // stats(decoded, instr);
  }

private:
  int32_t load_byte(uint32_t addr) {
    uint32_t data = (*_mem_interface).load_mem(addr, 1);
    data = data & 0x80 ? data | 0xffffff80 : data & 0x0000007f;
    return (int32_t)data;
  }
  int32_t load_half(uint32_t addr) {
    uint32_t data = (*_mem_interface).load_mem(addr, 2);
    data = data & 0x80 ? data | 0xffff8000 : data & 0x00007fff;
    return (int32_t)data;
  }
  int32_t load_word(uint32_t addr) {
    if (addr == 0x80000200) {
      printf("lw\n");
    }
    uint32_t data = (*_mem_interface).load_mem(addr, 4);
    return (int32_t)data;
  }
  uint32_t load_byte_unsigned(uint32_t addr) {
    uint32_t data = (*_mem_interface).load_mem(addr, 1);
    data = data & 0x000000ff;
    return (uint32_t)data;
  }
  uint32_t load_half_unsigned(uint32_t addr) {
    uint32_t data = (*_mem_interface).load_mem(addr, 2);
    data = data & 0x0000ffff;
    return (uint32_t)data;
  }

  void store_byte(uint32_t addr, uint32_t data) {
    (*_mem_interface).store_mem(addr, data, 1);
  }

  void store_half(uint32_t addr, uint32_t data) {
    (*_mem_interface).store_mem(addr, data, 2);
  }
  void store_word(uint32_t addr, uint32_t data) {
    (*_mem_interface).store_mem(addr, data, 4);
  }

  int call_exit(DecodedInstr *decoded, uint32_t instr) {
    std::cout << "----------------" << std::endl;
    std::cout << " invalid instr  " << std::endl;
    std::cout << "----------------" << std::endl;

    // uncomment the lines below for risc-v compliance test.
    if (debug) {
      uint32_t begin_signature_mem_ptr = load_word(BASE_ADDR + 0x20000 + 0x4);
      uint32_t end_signature_mem_ptr = load_word(BASE_ADDR + 0x20000 + 0x8);

      for (uint32_t i = begin_signature_mem_ptr; i < end_signature_mem_ptr;
           i += 4) {
        printf("%08x\n", load_word(i));
      }
    }
    exit(0);
    return 0;
  }
};