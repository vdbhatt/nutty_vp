#pragma once

#include "constants.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
class SOC_Memory : sc_core::sc_module {
public:
  SOC_Memory(const sc_core::sc_module_name &name,
             std::string initialization_file = "");
  ~SOC_Memory();

  tlm_utils::simple_target_socket<SOC_Memory> socket;
  virtual void b_tansport(tlm::tlm_generic_payload &trans,
                          sc_core::sc_time &delay);
  std::string instance_name;
  std::string init_file;

private:
  void _init_memory();
  char mem[MEM_SIZE];
  std::shared_ptr<spdlog::logger> logger;
};
