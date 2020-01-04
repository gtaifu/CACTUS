#include "fast_conditional_execution.h"

namespace cactus {

void Fast_conditional_execution::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "meas_ena_cancel");
}

Fast_conditional_execution::Fast_conditional_execution(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    open_telf_file();

    // output measure issue
    SC_CTHREAD(do_output, in_50MHz_clock.pos());

    // output log
    SC_CTHREAD(log_telf, in_50MHz_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Fast_conditional_execution::~Fast_conditional_execution() { close_telf_file(); }

void Fast_conditional_execution::do_output() {

    auto logger = get_logger_or_exit("console");

    Q_pipe_interface  q_pipe_interface;
    Generic_meas_if   meas;
    std::vector<bool> vec_meas_ena_cancel;

    while (true) {
        wait();

        // clear meas info
        meas.reset();
        vec_meas_ena_cancel.clear();

        // when received reset sig
        if (reset.read()) {
            // reset meas_ena_cancel
            for (size_t i = 0; i < m_num_qubits; ++i) {
                vec_meas_ena_cancel.push_back(false);
            }
            meas.set_meas_ena_cancel(vec_meas_ena_cancel);
            out_Qp2MRF_meas_cancel.write(meas);

            // reset operations
            q_pipe_interface.reset();
            out_q_pipe_interface.write(q_pipe_interface);

            continue;
        }

        q_pipe_interface = in_q_pipe_interface.read();

        // traverse each qubit operation to find measurement operation
        for (size_t i = 0; i < q_pipe_interface.ops.size(); ++i) {

            meas.timing = q_pipe_interface.timing;

            // TODO: currently fast condition execution has not been designed yet
            // if execute condition on i-th qubit which has a measurement operation is not
            // satisfied,then meas_ena_cancel should be set false on i-th qubit
            // it will be done in future work. we set it false just for now.
            vec_meas_ena_cancel.push_back(false);
        }

        meas.set_meas_ena_cancel(vec_meas_ena_cancel);
        out_Qp2MRF_meas_cancel.write(meas);

        // TODO: currently fast condition execution has not been designed yet
        // if execute condition on i-th qubit is not satisfied,then cancel i-th qubit operation
        // it will be done in future work. We make it straightforward just for now.
        out_q_pipe_interface.write(q_pipe_interface);
    }
}

void Fast_conditional_execution::log_telf() {

    Generic_meas_if   meas;
    std::vector<bool> vec_meas_ena_cancel;
    bool              meas_issue_cancel;

    while (true) {

        wait();

        if (is_telf_on) {

            // clear previous data
            vec_meas_ena_cancel.clear();

            meas                = out_Qp2MRF_meas_cancel.read();
            vec_meas_ena_cancel = meas.get_meas_ena_cancel();

            meas_issue_cancel = 0;
            for (size_t i = 0; i < vec_meas_ena_cancel.size(); ++i) {
                meas_issue_cancel |= vec_meas_ena_cancel[i];
            }

            if (meas_issue_cancel) {
                // telf_os << meas;
                telf_os << std::setfill(' ') << std::setw(12) << meas.timing.label;
                for (size_t i = 0; i < vec_meas_ena_cancel.size(); ++i) {
                    telf_os << " " << std::setfill(' ') << std::setw(3) << vec_meas_ena_cancel[i];
                }
                telf_os << std::endl;
            }
        }
    }
}

void Fast_conditional_execution::add_telf_header() {
    telf_os << std::setfill(' ') << std::setw(12) << "timing_label";
    for (unsigned int i = 0; i < m_num_qubits; ++i) {
        std::string qubit = "q" + to_string(i);
        telf_os << " " << std::setfill(' ') << std::setw(3) << qubit;
    }
    telf_os << std::endl;
}

void Fast_conditional_execution::add_telf_line() {}

}  // namespace cactus