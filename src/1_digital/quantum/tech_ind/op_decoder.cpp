#include "op_decoder.h"

namespace cactus {
void Op_decoder::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
}

Op_decoder::Op_decoder(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    SC_THREAD(do_output);
    sensitive << in_q_pipe_interface;

    logger->trace("Finished initializing {}...", this->name());
}

void Op_decoder::do_output() {

    Q_pipe_interface q_pipe_interface;

    while (true) {
        wait();

        // TODO: operation from one representation to another
        // currently, straight with input
        out_q_pipe_interface.write(in_q_pipe_interface.read());
    }
}

void Op_decoder::add_telf_header() {}

void Op_decoder::add_telf_line() {}

}  // end of namespace cactus
