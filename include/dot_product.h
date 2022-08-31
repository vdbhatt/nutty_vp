#pragma once

#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

class DotProduct : public sc_core::sc_module {
public:
  // 1 PORT FOR WRITING TO REGISTERS
  // 1 PORT FOR READING DATA
  // 1 PORT FOR RAISING THE INTERRUPT FOR CPU

  tlm_utils::simple_initiator_socket<DotProduct> data_socket;
  tlm_utils::simple_initiator_socket<DotProduct> interrupt_socket;
  tlm_utils::simple_target_socket<DotProduct> config_socket;
  uint32_t vector_a_start;
  uint32_t vector_b_start;
  uint32_t vector_length;
  uint32_t output_mac_address;

  uint32_t command_register;
  int mac_result;
  void thread_process();
  void interrupt_process();
  bool start_mac;
  sc_core::sc_event mac_done;
  DotProduct(const sc_core::sc_module_name &name);
  void config_b_transport(tlm::tlm_generic_payload &, sc_core::sc_time &);

private:
  int get_data(uint32_t, int, sc_core::sc_time &);
  void write_mac_data(sc_core::sc_time &delay);
};