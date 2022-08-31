#include "simple_memory_interface.h"

MemoryInterface::MemoryInterface(const sc_core::sc_module_name &name)
    : sc_module(name), socket("socket") {}
MemoryInterface::~MemoryInterface() {}

uint32_t MemoryInterface::load_mem(uint32_t addr, int length) {
  uint32_t data_to_return;
  tlm::tlm_generic_payload trans;
  auto delay = sc_core::sc_time(sc_core::SC_ZERO_TIME);
  trans.set_command(tlm::TLM_READ_COMMAND);
  trans.set_address(addr);
  trans.set_data_length(length);
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&data_to_return));
  trans.set_byte_enable_ptr(nullptr);
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_streaming_width(4);
  trans.set_dmi_allowed(false);
  socket->b_transport(trans, delay);
  wait(delay);
  if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
    return data_to_return;
  } else {
    SC_REPORT_ERROR("read_fail", "failed to read data");
    return 0;
  }
}

void MemoryInterface::store_mem(uint32_t addr, uint32_t data, int length) {
  tlm::tlm_generic_payload trans;
  auto delay = sc_core::sc_time(sc_core::SC_ZERO_TIME);
  trans.set_command(tlm::TLM_WRITE_COMMAND);
  trans.set_address(addr);
  trans.set_data_length(length);
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&data));
  trans.set_byte_enable_ptr(nullptr);
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_streaming_width(4);
  trans.set_dmi_allowed(false);
  socket->b_transport(trans, delay);
  wait(delay);
  if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
    return;
  } else {
    SC_REPORT_ERROR("read_fail", "failed to read data");
    return;
  }
}