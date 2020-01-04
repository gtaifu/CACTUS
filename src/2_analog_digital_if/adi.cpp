#include "adi.h"

#include "num_util.h"

namespace cactus {

void Analog_digital_if::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits      = global_config.num_qubits;
    m_qubit_simulator = global_config.qubit_simulator;

    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "adi");
    is_telf_on = true;
}

// instance a convert method, could be specified in configure file
void Analog_digital_if::instance_adi_convert() {

    auto logger = get_logger_or_exit("console");

    if (m_qubit_simulator == Qubit_simulator_type::QUANTUMSIM) {
        // instance quantumsim simulator
        p_adi_convert = new Adi_convert_to_quantumsim("adi_convert");
    } else if (m_qubit_simulator == Qubit_simulator_type::QICIRCUIT) {
        // instance circuit simulator
        p_adi_convert = new Adi_convert_to_circuit("circuit");
    } else {
        logger->error("{}: Cannot connect to a unrecognized qubit simulator. Simulation aborts!",
                      this->name());
        exit(EXIT_FAILURE);
    }
}

Analog_digital_if::Analog_digital_if(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , msmt_result_gen("msmt_result_gen") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    open_telf_file();

    // instance sub module
    instance_adi_convert();

    // ------------------------------------------------------------------------------------------
    // signal convert
    // ------------------------------------------------------------------------------------------
    // input
    p_adi_convert->in_clock(in_clock);
    p_adi_convert->reset(reset);
    p_adi_convert->in_50MHz_clock(in_50MHz_clock);
    p_adi_convert->in_q_pipe_interface(in_q_pipe_interface);

    // output
    p_adi_convert->ops_2_qsim(ops_2_qsim);

    // ------------------------------------------------------------------------------------------
    // Measurement result gen
    // ------------------------------------------------------------------------------------------
    // input
    msmt_result_gen.in_50MHz_clock(in_50MHz_clock);
    msmt_result_gen.reset(reset);
    msmt_result_gen.msmt_res(msmt_res);

    // output
    msmt_result_gen.out_meas_result(out_meas_result);

    // methods
    SC_CTHREAD(log_telf, in_50MHz_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Analog_digital_if::~Analog_digital_if() {
    close_telf_file();
    if (p_adi_convert) {
        delete p_adi_convert;
        p_adi_convert = nullptr;
    }
}

void Analog_digital_if::log_telf() {
    while (true) {

        wait();

        if (is_telf_on) {
            add_telf_line();
        }
    }
}

void Analog_digital_if::add_telf_header() {
    // Specify the signal type
    telf_os << "#ADI Module" << std::endl;

    telf_os << "Clock cycle,     Operation type";
    for (unsigned int i = 0; i < m_num_qubits; i++) {
        telf_os << ",         qubit" << i;
    }
    telf_os << std::endl;
}

void Analog_digital_if::add_telf_line() {
    Ops_2_qsim moment = ops_2_qsim.read();
    if (moment.triggered == true) {

        telf_os << moment;
        telf_os << std::endl;
    }
}

}  // namespace cactus
