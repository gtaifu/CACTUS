#ifndef _CCLIGHT_H_
#define _CCLIGHT_H_

#include <systemc>

#include "classical_top.h"
#include "generic_if.h"
#include "global_counter.h"
#include "msmt_result_analysis.h"
#include "q_data_type.h"
#include "quantum_pipeline.h"
#include "telf_module.h"

namespace cactus {

SC_MODULE(CC_Light) {
  public:  // IO ports
    // general input
    sc_in<bool> in_clock;
    sc_in<bool> in_50MHz_clock;
    sc_in<bool> reset;
    sc_in<bool> init;

    // classical pipeline interface to the user
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;
    sc_out<bool>                         Clp2App_done;
    sc_out<bool>                         Qp2App_eq_empty;

    // quantum-specific input from the user
    sc_in<bool> run;

    // device interface
    // analog digital interface -> CCL
    sc_in<Generic_meas_if> in_meas_result;

    // CCL -> analog digital interface
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:
    void init_mem_asm(std::string asm_fn);

  private:  // internal modules
    Classical_part       classical_top;
    Quantum_pipeline     quantum_top;
    Msmt_result_analysis msmt_result_analysis;

  private:  // internal signals
    // classical pipeline -> quantum pipeline
    sc_signal<bool>                Qp2Clp_ready;
    sc_signal<bool>                Clp2Qp_valid;
    sc_signal<Qasm_instruction>    Clp2Qp_insn;
    sc_signal<sc_uint<INSN_WIDTH>> Clp2Qp_Rs;

    // quantum pipeline -> classical pipeline
    sc_signal<Generic_meas_if> Qp2MRF_meas_issue;
    sc_signal<Generic_meas_if> Qp2MRF_meas_cancel;
    sc_signal<Generic_meas_if> Qp2MRF_meas_cancel_50MHz;

    // msmt_result_analysis -> classical pipeline & exe_flag_gen
    sc_signal<Generic_meas_if> Qm2MRF_meas_result;

  protected:
    unsigned int m_num_qubits;

    void config();

  public:
    CC_Light(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(CC_Light);
};

}  // namespace cactus

#endif  // _CCLIGHT_NEW_H_
