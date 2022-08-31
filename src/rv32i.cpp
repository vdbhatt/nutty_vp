#include "rv32i.h"
#include "systemc"
using namespace std;
using namespace sc_core;
using namespace sc_dt;

SC_HAS_PROCESS(RV32I);
RV32I::RV32I(const sc_core::sc_module_name &name)
    : sc_module(name), imem_socket("imem_socket"),
      interrupt_socket("interrupt_socket") {
  mem_interface = new MemoryInterface("dmem_interface");
  rv32i_iss = new RV32I_ISS(mem_interface);

  SC_THREAD(thread_process);
  for (int i = 0; i < 32; ++i)
    rv32i_iss->regs[i] = 0;
  for (int i = 0; i < 4096; ++i) {
    rv32i_iss->csr[i] = 0;
  }
  rv32i_iss->regs[2] = BASE_ADDR + MEM_SIZE - 4;
  rv32i_iss->regs[3] = BASE_ADDR;

  interrupt_socket.register_b_transport(this, &RV32I::b_transport);
}

void RV32I::b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
  std::cout << "@" << sc_time_stamp() << "interrupt received! \n";
  // auto t = rv32i_iss->csr[mstatus];
  // check for delay
  auto t = rv32i_iss->csr[mip];
  if ((t & mip_meip) == 0) {
    t = t | mip_meip;
    rv32i_iss->csr[mip] = t;
    auto old_pc = rv32i_iss->pc;
    rv32i_iss->csr[mepc] = old_pc;
    rv32i_iss->csr[mcause] = 0x8000000b;
    auto new_pc = rv32i_iss->csr[mtvec] - 4; // compensate for +4
    rv32i_iss->pc = new_pc;
  }
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
}
void RV32I::thread_process() {
  tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;
  tlm::tlm_generic_payload *instr_trans = new tlm::tlm_generic_payload;

  uint32_t current_instr;
  instr_trans->set_command(tlm::TLM_READ_COMMAND);
  instr_trans->set_data_ptr(reinterpret_cast<unsigned char *>(&current_instr));
  instr_trans->set_data_length(4);
  instr_trans->set_streaming_width(4);
  instr_trans->set_byte_enable_ptr(nullptr);
  instr_trans->set_dmi_allowed(false);
  instr_trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  while (true) {
    sc_time delay(0, SC_NS);
    instr_trans->set_address(rv32i_iss->pc);
    imem_socket->b_transport(*instr_trans, delay);
    rv32i_iss->execute(current_instr);
    delay += sc_time(1, SC_NS);
    wait(delay);
    std::cout << "@" << sc_core::sc_time_stamp() << " INSTR : 0x" << std::hex
              << rv32i_iss->pc << ": 0x" << std::hex << current_instr << "\n";
    rv32i_iss->pc += 4;
  }
}
RV32I::~RV32I() {}
