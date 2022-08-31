#include "bus_interconnect.h"
#include "rv32i.h"
#include "soc_memory.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "systemc"
#include "tlm.h"
#include "top.h"
#include <iostream>

std::shared_ptr<spdlog::logger> logger;
bool debug = false;
int sc_main(int argc, char *argv[]) {
  spdlog::filename_t filename = SPDLOG_FILENAME_T("simple_kernel.txt");
  logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("dumb_logger",
                                                             filename);
  logger->set_pattern("%v");
  logger->set_level(spdlog::level::info);
  logger->info("timestamp, R/W address, data");
  // Console logger with color
  std::string elf;
  if (argc == 2) {
    elf = argv[1];
  } else {
    elf = "/workspaces/nutty_vp/test/fw/build/nutty.elf.bin";
  }
  if (elf.compare("debug") == 0) {
    debug = true;
    elf = "/workspaces/nutty_vp/test/add-01.elf.bin";
  }
  Top top("top", elf);
  sc_start();
  return 0;
}