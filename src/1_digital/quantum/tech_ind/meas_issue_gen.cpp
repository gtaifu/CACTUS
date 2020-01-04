#include "meas_issue_gen.h"

namespace cactus {

void Meas_issue_gen::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "meas_issue_gen");
}

Meas_issue_gen::Meas_issue_gen(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    open_telf_file();

    // output measure issue
    SC_CTHREAD(do_output, in_clock.pos());

    // output log
    SC_CTHREAD(log_telf, in_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Meas_issue_gen::~Meas_issue_gen() { close_telf_file(); }

void Meas_issue_gen::do_output() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface  q_pipe_interface;
    Generic_meas_if   meas;
    std::vector<bool> vec_meas_ena;

    while (true) {
        wait();

        // clear meas info
        meas.reset();
        vec_meas_ena.clear();

        // when received reset sig
        if (reset.read()) {
            // reset meas_ena
            for (size_t i = 0; i < m_num_qubits; ++i) {
                vec_meas_ena.push_back(false);
            }
            meas.set_meas_ena(vec_meas_ena);
            out_Qp2MRF_meas_issue.write(meas);

            continue;
        }

        q_pipe_interface = in_q_pipe_interface.read();

        // iterate each qubit operation to find measurement operation
        for (size_t i = 0; i < q_pipe_interface.ops.size(); ++i) {

            meas.timing = q_pipe_interface.timing;
            // representation is opcode
            if (q_pipe_interface.ops[i].op.type == REPR_OPCODE) {
                if (q_pipe_interface.ops[i].op.opcode == 6) {
                    vec_meas_ena.push_back(true);
                } else {
                    vec_meas_ena.push_back(false);
                }
            } else if (q_pipe_interface.ops[i].op.type == REPR_NAME) {
                // representation is name
                if ((q_pipe_interface.ops[i].op.name.find("Meas") !=
                     q_pipe_interface.ops[i].op.name.npos) ||
                    (q_pipe_interface.ops[i].op.name.find("meas") !=
                     q_pipe_interface.ops[i].op.name.npos)) {
                    vec_meas_ena.push_back(true);
                } else {
                    vec_meas_ena.push_back(false);
                }
            } else if (q_pipe_interface.ops[i].op.type == REPR_UNDEF) {
                // invalid operation
                vec_meas_ena.push_back(false);
            } else {
                // other representation type is not support now
                logger->error(
                  "{}: Representation type {} is not support currently. Simulation Aborts!",
                  this->name(), q_pipe_interface.ops[i].op.type);
                exit(EXIT_FAILURE);
            }
        }  // end of iterate each qubit operation to find measurement operation

        meas.set_meas_ena(vec_meas_ena);
        out_Qp2MRF_meas_issue.write(meas);
    }
}

void Meas_issue_gen::log_telf() {

    Generic_meas_if   meas;
    std::vector<bool> vec_meas_ena;
    bool              meas_issue;

    while (true) {

        wait();

        if (is_telf_on) {

            // clear previous data
            vec_meas_ena.clear();

            meas         = out_Qp2MRF_meas_issue.read();
            vec_meas_ena = meas.get_meas_ena();

            meas_issue = 0;
            for (size_t i = 0; i < vec_meas_ena.size(); ++i) {
                meas_issue |= vec_meas_ena[i];
            }

            if (meas_issue) {
                // telf_os << meas;
                telf_os << std::setfill(' ') << std::setw(12) << meas.timing.label;
                for (size_t i = 0; i < vec_meas_ena.size(); ++i) {
                    telf_os << " " << std::setfill(' ') << std::setw(3) << vec_meas_ena[i];
                }
                telf_os << std::endl;
            }
        }
    }
}

void Meas_issue_gen::add_telf_header() {
    telf_os << std::setfill(' ') << std::setw(12) << "timing_label";
    for (unsigned int i = 0; i < m_num_qubits; ++i) {
        std::string qubit = "q" + to_string(i);
        telf_os << " " << std::setfill(' ') << std::setw(3) << qubit;
    }
    telf_os << std::endl;
}

void Meas_issue_gen::add_telf_line() {}

}  // namespace cactus