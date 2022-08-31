#include "bus_interconnect.h"

BusInterconnect::BusInterconnect(const sc_module_name &name)
    : sc_module(name), target_socket("target_socket"),
      dmem_target_socket("dmem_target_socket"),
      instr_memory_socket("instr_memory_socket"),
      data_memory_socket("data_memory_socket") {
  target_socket.register_b_transport(this, &BusInterconnect::b_transport);
  dmem_target_socket.register_b_transport(this,
                                          &BusInterconnect::dmem_b_transport);
  dotp_target_socket.register_b_transport(this,
                                          &BusInterconnect::dotp_b_transport);
}

void BusInterconnect::b_transport(tlm::tlm_generic_payload &trans,
                                  sc_time &delay) {
  sc_dt::uint64 address = trans.get_address();
  sc_dt::uint64 masked_address;

  unsigned int target_nr = decode_address(address, masked_address);
  trans.set_address(masked_address);
  switch (target_nr) {
  case 0:
    instr_memory_socket->b_transport(trans, delay);
    break;
  case 1:
    data_memory_socket->b_transport(trans, delay);
    break;
  default:
    SC_REPORT_ERROR("address", "address range didn't match");
    break;
  }
}

void BusInterconnect::dmem_b_transport(tlm::tlm_generic_payload &trans,
                                       sc_time &delay) {
  sc_dt::uint64 address = trans.get_address();
  sc_dt::uint64 masked_address;

  unsigned int target_nr = decode_address(address, masked_address);
  trans.set_address(masked_address);
  switch (target_nr) {
  case 0:
    instr_memory_socket->b_transport(trans, delay);
    break;
  case 1:
    if ((masked_address >= 0x80000) & (masked_address < 0x80010)) {
      dotp_socket->b_transport(trans, delay);
    } else {
      data_memory_socket->b_transport(trans, delay);
    }
    break;
  default:
    SC_REPORT_ERROR("address", "address range didn't match");
    break;
  }
}

void BusInterconnect::dotp_b_transport(tlm::tlm_generic_payload &trans,
                                       sc_time &delay) {
  sc_dt::uint64 address = trans.get_address();
  sc_dt::uint64 masked_address;
  masked_address = address & 0xFFFFF;
  trans.set_address(masked_address);
  data_memory_socket->b_transport(trans, delay);
}
