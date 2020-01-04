#include "q_tech_dep.h"

namespace cactus {

void Q_tech_dep::config() {
    // reserved for future global parameter setting
}

Q_tech_dep::Q_tech_dep(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , timing_control_unit("timing_control_unit")
    , fast_cond_exe("fast_conditional_execution") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    // ------------------------------------------------------------------------------------------
    // timing control unit
    // ------------------------------------------------------------------------------------------
    // input
    timing_control_unit.in_clock(in_clock);
    timing_control_unit.reset(reset);
    timing_control_unit.in_50MHz_clock(in_50MHz_clock);
    timing_control_unit.run(run);
    timing_control_unit.in_q_pipe_interface(in_q_pipe_interface);

    // output
    timing_control_unit.out_eq_almostfull(out_eq_almostfull);
    timing_control_unit.out_eq_empty(out_eq_empty);
    timing_control_unit.out_Qp2clp_ready(out_Qp2clp_ready);

    timing_control_unit.out_q_pipe_interface(q_pipe_interface_sig);

    // ------------------------------------------------------------------------------------------
    // fast conditional execution
    // ------------------------------------------------------------------------------------------
    // input
    fast_cond_exe.in_50MHz_clock(in_50MHz_clock);
    fast_cond_exe.reset(reset);
    fast_cond_exe.in_q_pipe_interface(q_pipe_interface_sig);

    // output
    fast_cond_exe.out_q_pipe_interface(out_q_pipe_interface);
    fast_cond_exe.out_Qp2MRF_meas_cancel(out_Qp2MRF_meas_cancel);

    logger->trace("Finished initializing {}...", this->name());
}

}  // end of namespace cactus