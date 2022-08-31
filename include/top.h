#pragma once

#include "bus_interconnect.h"
#include "dot_product.h"
#include "rv32i.h"
#include "soc_memory.h"
class Top : public sc_module {
public:
  RV32I cpu;
  BusInterconnect interconnect;
  SOC_Memory instr_memory;
  SOC_Memory data_memory;
  DotProduct dot_product;

  Top(const sc_module_name &name, string elf_path);
};