#pragma once
#include "rv32i_iss.h"
#include "simple_memory_interface.h"
#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

class RV32I : public sc_core::sc_module {
public:
  RV32I(const sc_core::sc_module_name &name);
  void thread_process();
  tlm_utils::simple_initiator_socket<RV32I> imem_socket;
  tlm_utils::simple_target_socket<RV32I> interrupt_socket;
  MemoryInterface *mem_interface;
  RV32I_ISS *rv32i_iss;
  ~RV32I();
  void b_transport(tlm::tlm_generic_payload &, sc_core::sc_time &);
};
