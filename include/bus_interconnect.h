#pragma once

#include "systemc"
#include "tlm.h"
#include "tlm_utils/convenience_socket_bases.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class BusInterconnect : public sc_module {
public:
  tlm_utils::simple_target_socket<BusInterconnect> target_socket;
  tlm_utils::simple_target_socket<BusInterconnect> dmem_target_socket;
  tlm_utils::simple_initiator_socket<BusInterconnect> instr_memory_socket;
  tlm_utils::simple_initiator_socket<BusInterconnect> data_memory_socket;
  tlm_utils::simple_initiator_socket<BusInterconnect> dotp_socket;
  tlm_utils::simple_target_socket<BusInterconnect> dotp_target_socket;

  BusInterconnect(const sc_module_name &);

  virtual void b_transport(tlm::tlm_generic_payload &, sc_time &);
  virtual void dmem_b_transport(tlm::tlm_generic_payload &trans,
                                sc_time &delay);
  void dotp_b_transport(tlm::tlm_generic_payload &, sc_time &);

  inline unsigned int decode_address(sc_dt::uint64 address,
                                     sc_dt::uint64 &masked_address) {
    unsigned int address_select =
        static_cast<unsigned int>((address >> 20) & 0xFFFFF);
    unsigned int target_nr;
    switch (address_select) {
    case 0x000: // imem
      target_nr = 0;
      break;
    case 0x800:
      target_nr = 1;
      break;
    case 0x810:
      target_nr = 2;
      break;
    default:
      SC_REPORT_ERROR("memory", "invalid address range");
      break;
    }
    masked_address = address & 0xFFFFF;
    return target_nr;
  }

  inline sc_dt::uint64 compose_address(unsigned int target_nr,
                                       sc_dt::uint64 masked_address) {
    switch (target_nr) {
    case 0:
      return masked_address;
    case 1:
      return (0x800 << 20) | (masked_address & 0xFFFFF);
    case 2:
      return (0x810 << 20) | (masked_address & 0xFFFFF);
    default:
      SC_REPORT_ERROR("memory", "invalid address range");
      break;
    }
  }
};