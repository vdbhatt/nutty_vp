#pragma once

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"

class MemoryInterface : public sc_core::sc_module {
public:
  tlm_utils::simple_initiator_socket<MemoryInterface> socket;
  MemoryInterface(const sc_core::sc_module_name &);
  ~MemoryInterface();
  uint32_t load_mem(uint32_t addr, int);
  void store_mem(uint32_t addr, uint32_t, int);
};