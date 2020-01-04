#include "timing_control_unit.h"

namespace cactus {

void Timing_control_unit::config() {
    // reverved for future global parameter setting
}

Timing_control_unit::Timing_control_unit(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , event_queue_manager("event_queue_manager") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    // ------------------------------------------------------------------------------------------
    // event queue manager
    // ------------------------------------------------------------------------------------------
    // input
    event_queue_manager.in_clock(in_clock);
    event_queue_manager.reset(reset);
    event_queue_manager.in_50MHz_clock(in_50MHz_clock);
    event_queue_manager.in_q_pipe_interface(in_q_pipe_interface);
    event_queue_manager.run(run);

    // output
    event_queue_manager.out_error_state(out_eq_empty);
    event_queue_manager.out_eq_almostfull(out_eq_almostfull);
    event_queue_manager.out_q_pipe_interface(out_q_pipe_interface);

    // initial signal
    out_Qp2clp_ready.initialize(true);

    // methods
    SC_THREAD(do_output);
    sensitive << out_eq_almostfull << reset;

    logger->trace("Finished initializing {}...", this->name());
}

// thread
void Timing_control_unit::do_output() {
    while (true) {
        wait();

        // whether quantum pipeline is ready
        if (reset.read()) {
            out_Qp2clp_ready = true;
        } else {
            out_Qp2clp_ready = !out_eq_almostfull.read();
        }
    }
}

}  // end of namespace cactus