#include "qvm.h"

namespace cactus {

void QVM::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits      = global_config.num_qubits;
    m_qubit_simulator = global_config.qubit_simulator;
}

void QVM::init_mem_asm(std::string asm_fn) { cclight.init_mem_asm(asm_fn); }

QVM::QVM(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , cclight("cclight")
    , adi("adi") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    // instance quantumsim or QIcircuit simulator
    if (m_qubit_simulator == Qubit_simulator_type::QUANTUMSIM) {
        p_quantumsim = new If_QuantumSim("if_quantumsim");
    } else if (m_qubit_simulator == Qubit_simulator_type::QICIRCUIT) {
        p_QIcircuit = new If_QIcircuit("if_QIcircuit");
    } else {
        logger->error("{}: Cannot instance an unknown qubit simulator '{}'. Simulation aborts!",
                      this->name(), m_qubit_simulator);
        exit(EXIT_FAILURE);
    }

    // ------------------------------------------------------------------------------------------
    // cclight
    // ------------------------------------------------------------------------------------------
    // input
    cclight.in_clock(clock);
    cclight.in_50MHz_clock(clock_50MHz);
    cclight.reset(reset);
    cclight.init(init);
    cclight.App2Clp_init_pc(App2Clp_init_pc);
    cclight.run(run);

    // interface to ADI
    cclight.in_meas_result(meas_result_sig);

    // interface to ADI
    cclight.out_q_pipe_interface(q_pipe_interface_sig);

    // output
    cclight.Clp2App_done(Clp2App_done);
    cclight.Qp2App_eq_empty(Qp2App_eq_empty);

    // ------------------------------------------------------------------------------------------
    // ADI
    // ------------------------------------------------------------------------------------------
    // input
    adi.in_clock(clock);
    adi.reset(reset);
    adi.in_50MHz_clock(clock_50MHz);

    // interface to ccl
    adi.in_q_pipe_interface(q_pipe_interface_sig);

    // interface to quantum sim
    adi.ops_2_qsim(ops_2_qsim);
    adi.msmt_res(msmt_res);

    // interface to ccl
    adi.out_meas_result(meas_result_sig);

    // ------------------------------------------------------------------------------------------
    // quantum simulator
    // ------------------------------------------------------------------------------------------
    if (m_qubit_simulator == Qubit_simulator_type::QUANTUMSIM) {
        // input
        p_quantumsim->clock_50MHz(clock_50MHz);
        p_quantumsim->init(init);
        p_quantumsim->ops_2_qsim(ops_2_qsim);

        // interface to ADI
        p_quantumsim->msmt_res(msmt_res);
    } else if (m_qubit_simulator == Qubit_simulator_type::QICIRCUIT) {
        // input
        p_QIcircuit->clock_50MHz(clock_50MHz);
        p_QIcircuit->init(init);
        p_QIcircuit->ops_2_qsim(ops_2_qsim);

        // interface to ADI
        p_QIcircuit->msmt_res(msmt_res);
    } else {
    }

    logger->trace("Finished initializing {}...", this->name());
}

}  // namespace cactus
