#include "quantum_pipeline.h"

namespace cactus {

void Quantum_pipeline::config() {
    Global_config& global_config = Global_config::get_instance();
    m_num_qubits                 = global_config.num_qubits;
}

Quantum_pipeline::Quantum_pipeline(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , q_ppl_ind("q_ppl_ind")
    , q_ppl_dep("q_ppl_dep") {

    auto logger = get_logger_or_exit("console");

    logger->trace("Start initializing {}...", this->name());

    config();

    // IO vector signals

    // ----------------------------------------------------------------------------------------
    // quantum pipeline, technology independent part
    // ----------------------------------------------------------------------------------------
    // input
    q_ppl_ind.in_clock(in_clock);
    q_ppl_ind.reset(reset);
    q_ppl_ind.in_bundle(Clp2Qp_insn);
    q_ppl_ind.in_valid_bundle(Clp2Qp_valid);
    q_ppl_ind.in_rs_wait_time(Clp2Qp_Rs);

    // output
    q_ppl_ind.out_Qp2MRF_meas_issue(out_Qp2MRF_meas_issue);
    q_ppl_ind.out_q_pipe_interface(q_pipe_interface_sig);

    // ----------------------------------------------------------------------------------------
    // quantum pipeline, technology dependent part
    // ----------------------------------------------------------------------------------------
    // input
    q_ppl_dep.in_clock(in_clock);
    q_ppl_dep.reset(reset);
    q_ppl_dep.in_50MHz_clock(in_50MHz_clock);
    q_ppl_dep.run(run);
    q_ppl_dep.in_q_pipe_interface(q_pipe_interface_sig);
    // output
    q_ppl_dep.out_q_pipe_interface(out_q_pipe_interface);
    q_ppl_dep.out_eq_empty(out_eq_empty);
    q_ppl_dep.out_eq_almostfull(eq_almostfull_sig);
    q_ppl_dep.out_Qp2clp_ready(Qp2Clp_ready);
    q_ppl_dep.out_Qp2MRF_meas_cancel(out_Qp2MRF_meas_cancel);

    logger->trace("Finished initializing {}...", this->name());
}

}  // end of namespace cactus
