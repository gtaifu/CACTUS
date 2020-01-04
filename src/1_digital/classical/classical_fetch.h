#ifndef _CLASSICAL_FETCH_H_
#define _CLASSICAL_FETCH_H_

#include <systemc>

#include "global_counter.h"
#include "global_json.h"
#include "q_data_type.h"
#include "qasm_instruction.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_int;
using sc_dt::sc_uint;

class Classical_fetch : public Telf_module {
  public:
    sc_in<bool> clock;
    sc_in<bool> reset;

    // interface to the user
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;

    // quantum pipeline is ready or not.
    sc_in<bool> Qp2Clp_ready;

    // instructions sent from icache to classical fetch
    sc_in<bool>             IC2Clp_valid;
    sc_in<bool>             IC2Clp_branch;
    sc_in<Qasm_instruction> insn;

    // feedback control related interface
    // decode stage -> fetch stage
    sc_in<bool>                          de_run;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> de_pc;
    sc_in<bool>                          de_reset;
    sc_in<bool>                          de_reset_dly;
    // de_clk_en: signal indicates that the pipeline is stalled or not
    sc_in<bool>                          de_clk_en;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> de_init_pc;
    sc_in<bool>                          de_br_start;

    // fetch stage -> decode stage
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> if_init_pc;
    sc_out<bool>                          if_reset;
    sc_out<bool>                          if_reset_dly;
    sc_out<bool>                          if_br_done;
    sc_out<bool>                          if_br_done_valid;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> if_normal_pc;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> if_target_pc;
    sc_out<Qasm_instruction>              de_insn;
    sc_out<bool>                          de_insn_valid;
    sc_out<bool>                          if_run;
    sc_out<bool>                          de_qp_ready;

  public:  // methods
    void signal_update();
    void pc_adder();
    void start_up_logic();
    void branch_target_adder();
    void branch_latency_control();

  public:
    void config();

  public:
    Classical_fetch(const sc_core::sc_module_name& n);

    ~Classical_fetch();

    SC_HAS_PROCESS(Classical_fetch);
};

}  // namespace cactus

#endif  //_CLASSICAL_FETCH_H_
