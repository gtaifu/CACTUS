#include "msmt_result_analysis.h"

#include <vector>

namespace cactus {

void Msmt_result_analysis::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
}

Msmt_result_analysis::Msmt_result_analysis(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , meas_cancel_fifo(MEAS_CANCEL_FIFO_DEPTH)
    , meas_result_fifo(MEAS_RESULT_FIFO_DEPTH) {

    auto logger = get_logger_or_exit("console");
    logger->debug("Start initializing {}...", this->name());

    config();

    // buffered Qp2MRF_meas_cancel signal
    SC_CTHREAD(write_meas_cancel_fifo, in_50MHz_clock.pos());
    SC_CTHREAD(read_meas_cancel_fifo, in_clock.pos());

    // buffered Qm2MRF_meas_result signal
    SC_CTHREAD(write_meas_result_fifo, in_50MHz_clock.pos());
    SC_CTHREAD(read_meas_result_fifo, in_clock.pos());

    logger->debug("Finished initializing {}...", this->name());
}

Msmt_result_analysis::~Msmt_result_analysis() { close_telf_file(); }

void Msmt_result_analysis::write_meas_cancel_fifo() {

    auto logger = get_logger_or_exit("console");

    while (true) {
        wait();

        if (meas_cancel_fifo.num_available() > MEAS_CANCEL_FIFO_DEPTH) {
            logger->error(
              "{}: Measurement cancel fifo cannot write any more data while it has been full "
              "already.Simulation aborts!",
              this->name());
            exit(EXIT_FAILURE);
        } else {
            meas_cancel_fifo.write(in_Qp2MRF_meas_cancel.read());
        }
    }
}

void Msmt_result_analysis::read_meas_cancel_fifo() {

    std::vector<bool> vec_meas_cancel;
    Generic_meas_if   meas;

    while (true) {
        wait();

        if (meas_cancel_fifo.num_available() == 0) {
            // default output when fifo is empty
            meas.reset();
            vec_meas_cancel.clear();
            for (size_t i = 0; i < m_num_qubits; ++i) {
                vec_meas_cancel.push_back(false);
            }

            meas.set_meas_ena_cancel(vec_meas_cancel);
            out_Qp2MRF_meas_cancel.write(meas);
        } else {
            out_Qp2MRF_meas_cancel.write(meas_cancel_fifo.read());
        }
    }
}

void Msmt_result_analysis::write_meas_result_fifo() {

    auto logger = get_logger_or_exit("console");

    while (true) {
        wait();

        if (meas_result_fifo.num_available() > MEAS_RESULT_FIFO_DEPTH) {
            logger->error(
              "{}: Measurement result fifo cannot write any more data while it is full "
              "already.Simulation aborts!",
              this->name());
            exit(EXIT_FAILURE);
        } else {
            meas_result_fifo.write(in_meas_result.read());
        }
    }
}

void Msmt_result_analysis::read_meas_result_fifo() {

    std::vector<bool> vec_meas_data;
    std::vector<bool> vec_meas_data_valid;
    Generic_meas_if   meas;

    while (true) {
        wait();

        if (meas_result_fifo.num_available() == 0) {
            // default output when fifo is empty
            meas.reset();
            vec_meas_data.clear();
            vec_meas_data_valid.clear();
            for (size_t i = 0; i < m_num_qubits; ++i) {
                vec_meas_data.push_back(false);
                vec_meas_data_valid.push_back(false);
            }

            meas.set_meas_data(vec_meas_data);
            meas.set_meas_data_valid(vec_meas_data_valid);
            out_Qm2MRF_meas_result.write(meas);
        } else {
            out_Qm2MRF_meas_result.write(meas_result_fifo.read());
        }
    }
}

}  // namespace cactus
