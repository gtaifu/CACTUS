#include "cclight_new.h"

namespace cactus {

void CC_Light::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
}

void CC_Light::init_mem_asm(std::string asm_fn) { classical_top.init_mem_asm(asm_fn); }

CC_Light::CC_Light(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , classical_top("classical")
    , quantum_top("quantum")
    , msmt_result_analysis("msmt_result_analysis") {

    auto logger = get_logger_or_exit("console");
    logger->debug("Start initializing {}...", this->name());

    config();

    // ------------------------------------------------------------------------------------------
    // classical top
    // ------------------------------------------------------------------------------------------
    // general input
    classical_top.in_clock(in_clock);
    classical_top.reset(reset);

    // classical-specific input
    classical_top.init(init);
    classical_top.App2Clp_init_pc(App2Clp_init_pc);
    classical_top.Clp2App_done(Clp2App_done);

    // interface to quantum top
    classical_top.Qp2Clp_ready(Qp2Clp_ready);
    classical_top.Clp2Qp_valid(Clp2Qp_valid);
    classical_top.Clp2Qp_insn(Clp2Qp_insn);
    classical_top.Clp2Qp_Rs(Clp2Qp_Rs);

    // interface to measurement result analysis
    classical_top.Qp2MRF_meas_issue(Qp2MRF_meas_issue);
    classical_top.Qm2MRF_meas_result(Qm2MRF_meas_result);
    classical_top.Qp2MRF_meas_cancel(Qp2MRF_meas_cancel);

    // ------------------------------------------------------------------------------------------
    // quantum pipeline top
    // ------------------------------------------------------------------------------------------
    // general input
    quantum_top.in_clock(in_clock);
    quantum_top.in_50MHz_clock(in_50MHz_clock);
    quantum_top.reset(reset);

    // quantum-specific input
    quantum_top.run(run);

    // interface to classical top
    quantum_top.Qp2Clp_ready(Qp2Clp_ready);
    quantum_top.Clp2Qp_valid(Clp2Qp_valid);
    quantum_top.Clp2Qp_insn(Clp2Qp_insn);
    quantum_top.Clp2Qp_Rs(Clp2Qp_Rs);

    // output
    // quantum pipeline -> analog digital interface
    quantum_top.out_q_pipe_interface(out_q_pipe_interface);

    // quantum pipeline -> classical pipeline
    quantum_top.out_Qp2MRF_meas_cancel(Qp2MRF_meas_cancel_50MHz);
    quantum_top.out_Qp2MRF_meas_issue(Qp2MRF_meas_issue);

    // quantum pipeline -> CCL
    quantum_top.out_eq_empty(Qp2App_eq_empty);

    // ------------------------------------------------------------------------------------------
    // measurement result analysis
    // ------------------------------------------------------------------------------------------
    // input
    msmt_result_analysis.in_clock(in_clock);
    msmt_result_analysis.in_50MHz_clock(in_50MHz_clock);
    msmt_result_analysis.reset(reset);

    // interface to analog digital interface
    msmt_result_analysis.in_meas_result(in_meas_result);
    // interface to quantum pipeline
    msmt_result_analysis.in_Qp2MRF_meas_cancel(Qp2MRF_meas_cancel_50MHz);

    // output
    // interface to analog digital interface
    msmt_result_analysis.out_Qm2MRF_meas_result(Qm2MRF_meas_result);
    // interface to classical pipeline
    msmt_result_analysis.out_Qp2MRF_meas_cancel(Qp2MRF_meas_cancel);

    logger->debug("Finished initializing {}...", this->name());
}

}  // namespace cactus
