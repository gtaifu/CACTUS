#ifndef _QUANTUM_PIPELINE_H_
#define _QUANTUM_PIPELINE_H_

#include <systemc.h>

#include "generic_if.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "q_data_type.h"
#include "tech_dep/q_tech_dep.h"
#include "tech_ind/q_tech_ind.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Quantum_pipeline) {
  public:
    // input
    sc_in<bool> in_clock;
    sc_in<bool> in_50MHz_clock;
    sc_in<bool> reset;

    sc_in<bool>                run;
    sc_in<bool>                Clp2Qp_valid;
    sc_in<Qasm_instruction>    Clp2Qp_insn;
    sc_in<sc_uint<INSN_WIDTH>> Clp2Qp_Rs;

    // output
    // interface to ADI
    sc_out<Q_pipe_interface> out_q_pipe_interface;

    // interface to classical pipeline
    sc_out<Generic_meas_if> out_Qp2MRF_meas_issue;
    sc_out<Generic_meas_if> out_Qp2MRF_meas_cancel;

    // The quantum pipeline tells the classical pipeline if it can further accept new instructions
    // or not
    sc_out<bool> Qp2Clp_ready;
    sc_out<bool> out_eq_empty;

  protected:  // internal modules
    Q_tech_ind q_ppl_ind;
    Q_tech_dep q_ppl_dep;

  protected:  // internal signals
    sc_signal<Q_pipe_interface> q_pipe_interface_sig;

    sc_signal<bool> eq_almostfull_sig;

  protected:
    int m_num_qubits;

    void config();

  public:
    Quantum_pipeline(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Quantum_pipeline);
};
}  // namespace cactus

#endif
