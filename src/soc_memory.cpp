#include "soc_memory.h"
#include "constants.h"
#include "spdlog/fmt/bin_to_hex.h"
#include <fstream>
#include <string>
#include <vector>
SOC_Memory::SOC_Memory(const sc_core::sc_module_name &name,
                       std::string initialization_file)
    : sc_module(name), instance_name{name}, init_file{initialization_file} {
  socket.register_b_transport(this, &SOC_Memory::b_tansport);
  _init_memory();
  logger = spdlog::get("dumb_logger");
}

SOC_Memory::~SOC_Memory() { ; }

void SOC_Memory::_init_memory() {
  if (init_file.empty()) {
    for (size_t i = 0; i < MEM_SIZE / 4; i += 4) {
      mem[i] = 0xef;
      mem[i + 1] = 0xbe;
      mem[i + 2] = 0xad;
      mem[i + 3] = 0xde;
    }
  } else {
    std::ifstream ifd(init_file, std::ios::binary | std::ios::ate);
    int size = ifd.tellg();
    ifd.seekg(0, std::ios::beg);
    std::vector<char> buffer;
    buffer.resize(size);
    ifd.read(buffer.data(), size);
    for (std::size_t i = 0; i < buffer.size(); ++i) {
      mem[i] = (buffer[i] & 0xFF);
    }
    for (size_t i = buffer.size(); i < MEM_SIZE; ++i) {
      mem[i] = 0;
    }
  }
  std::cout << "INFO: memory initialized\n";
}

void SOC_Memory::b_tansport(tlm::tlm_generic_payload &trans,
                            sc_core::sc_time &delay) {
  auto cmd = trans.get_command();
  auto addr = trans.get_address();
  unsigned char *ptr = trans.get_data_ptr();
  auto length = trans.get_data_length();
  auto byte = trans.get_byte_enable_ptr();
  auto width = trans.get_streaming_width();
  if (addr > MEM_SIZE) {
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }
  if (byte != nullptr) {
    trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
    return;
  }
  if (length > 4 || width < length) {
    trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
    return;
  }
  uint32_t data = 0;
  if (cmd == tlm::TLM_READ_COMMAND) {
    memcpy(ptr, &mem[addr], length);
    memcpy(&data, &mem[addr], length);
    // std::cout << sc_core::sc_time_stamp() << " R : 0x" << std::hex << addr
    //           << " : 0x" << std::hex << data << "\n";
    logger->info("@{} ps. R {}. {}", sc_core::sc_time_stamp().value(), addr,
                 data);
  } else if (cmd == tlm::TLM_WRITE_COMMAND) {
    memcpy(&mem[addr], ptr, length);
    memcpy(&data, ptr, length);
    // std::cout << sc_core::sc_time_stamp() << " W : 0x" << std::hex << addr
    //           << " : 0x" << std::hex << data << "\n";
    logger->info("@{} ps. W {}. {}", sc_core::sc_time_stamp().value(), addr,
                 data);
  } else {
    SC_REPORT_ERROR("memory", "fault in b_transport of memory");
  }
  trans.set_dmi_allowed(false);
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
  // add delay time to reflect read and write
  delay = delay + sc_core::sc_time(1, sc_core::SC_NS);
  std::cout << "@" << sc_core::sc_time_stamp() << " MEM : " << delay << " : "
            << " 0x" << std::hex << addr << " :0x" << std::hex << data << "\n";
  return;
}