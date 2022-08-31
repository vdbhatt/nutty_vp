#include "dot_product.h"
#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

SC_HAS_PROCESS(DotProduct);

int DotProduct::get_data(uint32_t address, int offset,
                         sc_core::sc_time &delay) {
  int data;
  tlm::tlm_generic_payload trans;
  sc_dt::uint64 addr = static_cast<sc_dt::uint64>(address + offset);
  trans.set_address(addr);
  trans.set_command(tlm::TLM_READ_COMMAND);
  trans.set_data_length(4);
  trans.set_dmi_allowed(false);
  trans.set_byte_enable_ptr(nullptr);
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&data));
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_streaming_width(4);
  data_socket->b_transport(trans, delay);
  delay += sc_core::sc_time(1, sc_core::SC_NS);
  if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
    return data;
  } else {
    SC_REPORT_ERROR("memory", "unable to read data");
  }
}
void DotProduct::thread_process() {
  int i = 0;
  sc_core::sc_time clk = sc_core::sc_time(1, sc_core::SC_NS);
  mac_result = 0;

  while (true) {
    sc_core::sc_time delay(1, sc_core::SC_NS);
    // ok we got start let's start reading data 4 bytes at a time and do the
    // dot product
    // std::cout << "@" << sc_core::sc_time_stamp() << " DOTP : "
    //           << " : " << start_mac << "\n";
    if (start_mac & (i < 4 * vector_length)) {
      // get the i'th value from the address
      int a = get_data(vector_a_start, i, delay);
      int b = get_data(vector_b_start, i, delay);
      mac_result += a * b;
      i += 4;
    }
    if (start_mac & (i >= 4 * vector_length)) {
      mac_done.notify(sc_core::SC_ZERO_TIME); // for next delta cycle}
      start_mac = false;
      i = 0;
    }
    wait(delay);
  }
}
void DotProduct::interrupt_process() {
  int data = 1; // dummy id
  tlm::tlm_generic_payload trans;
  sc_core::sc_time delay(1, sc_core::SC_NS);
  sc_dt::uint64 addr = static_cast<sc_dt::uint64>(1);
  trans.set_address(addr);
  trans.set_command(tlm::TLM_WRITE_COMMAND);
  trans.set_data_length(4);
  trans.set_dmi_allowed(false);
  trans.set_byte_enable_ptr(nullptr);
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&data));
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_streaming_width(4);
  delay += sc_core::sc_time(1, sc_core::SC_NS);

  while (true) {
    wait(mac_done);
    interrupt_socket->b_transport(trans, delay);
    if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
      std::cout << "@" << sc_core::sc_time_stamp()
                << " interrupt raised for mac done " << mac_result << "\n";
    } else {
      SC_REPORT_ERROR("memory", "unable to read data");
    }
  }
}

DotProduct::DotProduct(const sc_core::sc_module_name &name) : sc_module(name) {
  SC_THREAD(thread_process);
  SC_THREAD(interrupt_process);
  config_socket.register_b_transport(this, &DotProduct::config_b_transport);
  start_mac = false;
}

void DotProduct::config_b_transport(tlm::tlm_generic_payload &trans,
                                    sc_core::sc_time &delay) {
  auto cmd = trans.get_command();
  auto address = trans.get_address();
  auto ptr = trans.get_data_ptr();
  auto length = trans.get_data_length();
  auto byte = trans.get_byte_enable_ptr();
  auto dmi = trans.is_dmi_allowed();
  auto width = trans.get_streaming_width();

  if (cmd == tlm::TLM_WRITE_COMMAND) {
    // check for address
    if (address == 0x00080000) {
      // vector a base address register
      memcpy(&vector_a_start, ptr, length);
    }
    if (address == 0x00080004) {
      // vector b base address
      memcpy(&vector_b_start, ptr, length);
    }

    if (address == 0x00080008) {
      // length register
      memcpy(&vector_length, ptr, length);
    }
    if (address == 0x0008000c) {
      // command register
      memcpy(&command_register, ptr, length);
      if (command_register & 0x1) {
        start_mac = true;
        std::cout << "Starting mac \n";
      }
    }
    if (address == 0x0008000c) {
      // command register
      memcpy(&command_register, ptr, length);
      if (command_register & 0x10) { // read the mac_value
        start_mac = false;
        std::cout << "MAC value : " << mac_result
                  << " written to output address \n";
        write_mac_data(delay);
      }
    }
    if (address == 0x00080010) {
      // output address
      memcpy(&output_mac_address, ptr, length);
    }
  }
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
  delay += sc_core::sc_time(1, sc_core::SC_NS);
  return;
}

void DotProduct::write_mac_data(sc_core::sc_time &delay) {
  tlm::tlm_generic_payload trans;
  sc_dt::uint64 addr = static_cast<sc_dt::uint64>(output_mac_address);
  trans.set_address(addr);
  trans.set_command(tlm::TLM_WRITE_COMMAND);
  trans.set_data_length(4);
  trans.set_dmi_allowed(false);
  trans.set_byte_enable_ptr(nullptr);
  trans.set_data_ptr(reinterpret_cast<unsigned char *>(&mac_result));
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_streaming_width(4);
  data_socket->b_transport(trans, delay);
  delay = delay + sc_core::sc_time(1, sc_core::SC_NS);
  delay += sc_core::sc_time(1, sc_core::SC_NS);
  if (trans.get_response_status() == tlm::TLM_OK_RESPONSE) {
    return;
  } else {
    SC_REPORT_ERROR("memory", "unable to read data");
  }
}