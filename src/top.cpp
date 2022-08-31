#include "top.h"
#include "systemc"

SC_HAS_PROCESS(Top);
Top::Top(const sc_core::sc_module_name &name, std::string elf_path)
    : sc_module(name), cpu("cpu"), interconnect("interconnect"),
      instr_memory("instr_memory", elf_path), data_memory("data_memory"),
      dot_product("dot_product") {
  cpu.imem_socket.bind(interconnect.target_socket);
  interconnect.instr_memory_socket.bind(instr_memory.socket);

  cpu.mem_interface->socket.bind(interconnect.dmem_target_socket);
  interconnect.data_memory_socket.bind(data_memory.socket);

  interconnect.dotp_socket.bind(dot_product.config_socket);
  dot_product.data_socket.bind(interconnect.dotp_target_socket);
  dot_product.interrupt_socket.bind(cpu.interrupt_socket);
}