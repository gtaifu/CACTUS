#include "operation_combiner.h"

namespace cactus {

void Operation_combiner::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
    m_vliw_width = global_config.vliw_width;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "op_combine");
}

Operation_combiner::Operation_combiner(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();
    open_telf_file();

    // initial signals
    Q_pipe_interface q_pipe_interface;
    q_pipe_interface.ops.resize(m_num_qubits);
    q_pipe_interface_sig.write(q_pipe_interface);

    // initial I/O port
    vec_in_q_pipe_interface.init(m_vliw_width);

    SC_CTHREAD(do_output, in_clock.pos());

    SC_THREAD(detect_timestamp_match);
    sensitive << i_timestamp << reset;
    for (size_t i = 0; i < m_vliw_width; ++i) {
        sensitive << vec_in_q_pipe_interface[i];
    }

    SC_CTHREAD(log_telf, in_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Operation_combiner::~Operation_combiner() { close_telf_file(); }

void Operation_combiner::do_output() {

    auto logger = get_logger_or_exit("console");

    std::vector<Q_pipe_interface> vec_q_pipe_interface;
    Q_pipe_interface              q_pipe_interface;

    vec_q_pipe_interface.resize(m_vliw_width);

    while (true) {
        wait();

        q_pipe_interface.reset();  // clear the temp variable
        q_pipe_interface.ops.resize(m_num_qubits);

        if (reset.read()) {  // when received reset sig
            i_timestamp.write(0);
            out_q_pipe_interface.write(q_pipe_interface);
            q_pipe_interface_sig.write(q_pipe_interface);  // update cached data

            continue;
        }

        // merge each vliw pipelane
        // only need to care Q_pipe_interface member variable: if_content,timing,vliw_width,ops
        for (size_t i = 0; i < m_vliw_width; ++i) {
            vec_q_pipe_interface[i].reset();
            vec_q_pipe_interface[i] = vec_in_q_pipe_interface[i].read();

            q_pipe_interface.if_content = vec_q_pipe_interface[i].if_content;
            q_pipe_interface.timing     = vec_q_pipe_interface[i].timing;
            q_pipe_interface.vliw_width = vec_q_pipe_interface[i].vliw_width;

            for (size_t j = 0; j < vec_q_pipe_interface[i].ops.size(); ++j) {

                if (q_pipe_interface.ops[j].is_valid() &&
                    vec_q_pipe_interface[i]
                      .ops[j]
                      .is_valid()) {  // cause conflict when two operations on the same qubit
                    logger->error(
                      "{}: More than one qubit gate on qubit {} at timing label '0x{:x}'",
                      this->name(), j, q_pipe_interface.timing.label);
                    exit(EXIT_FAILURE);
                } else if (!q_pipe_interface.ops[j].is_valid() &&
                           vec_q_pipe_interface[i].ops[j].is_valid()) {

                    q_pipe_interface.ops[j] = vec_q_pipe_interface[i].ops[j];
                } else {
                    // if no valid operation on this qubit,do nothing
                }
            }
        }

        // check whether there is a valid qop when if_content.valid_qop == true
        bool check_valid_qop = false;
        for (size_t i = 0; i < q_pipe_interface.ops.size(); ++i) {
            if (q_pipe_interface.ops[i].is_valid()) {
                check_valid_qop = true;
                break;
            }
        }
        if (!check_valid_qop && q_pipe_interface.if_content.valid_qop) {
            logger->error(
              "{}: No qubit has been assigned at current quantum operation with timing label "
              "'0x{:x}'. Simulations aborts!",
              this->name(), q_pipe_interface.timing.label);
            exit(EXIT_FAILURE);
        }

        // do output when received next timing label
        if (!i_timestamp_match) {
            out_q_pipe_interface.write(q_pipe_interface_sig);
            i_timestamp.write(q_pipe_interface.timing.label);  // update timing label
            q_pipe_interface_sig.write(q_pipe_interface);      // update cached data
        } else {
            Q_pipe_interface tmp_q_pipe_interface;
            tmp_q_pipe_interface.ops.resize(m_num_qubits);

            out_q_pipe_interface.write(tmp_q_pipe_interface);  // default output

            tmp_q_pipe_interface = q_pipe_interface_sig.read();

            // merge next instruction which has the same timing label
            // only need to care Q_pipe_interface member variable: if_content,timing,ops
            tmp_q_pipe_interface.if_content.valid_qop |= q_pipe_interface.if_content.valid_qop;
            tmp_q_pipe_interface.if_content.valid_wait |= q_pipe_interface.if_content.valid_wait;

            if (tmp_q_pipe_interface.timing.label != q_pipe_interface.timing.label) {
                logger->error(
                  "{}: Error occurs when combining the operations at different instructions which "
                  "has the same timing label '0x{:x}'.",
                  this->name(), q_pipe_interface.timing.label);
                exit(EXIT_FAILURE);
            }

            for (size_t j = 0; j < m_num_qubits; ++j) {

                if (tmp_q_pipe_interface.ops[j].is_valid() &&
                    q_pipe_interface.ops[j]
                      .is_valid()) {  // cause conflict when two operations on the same qubit
                    logger->error("{}: Operations conflict on qubit {} at timing label '0x{:x}'",
                                  this->name(), j, q_pipe_interface.timing.label);
                    exit(EXIT_FAILURE);
                } else if (!tmp_q_pipe_interface.ops[j].is_valid() &&
                           q_pipe_interface.ops[j].is_valid()) {

                    tmp_q_pipe_interface.ops[j] = q_pipe_interface.ops[j];
                } else {
                    // if no valid operation on this qubit,do nothing
                }
            }

            q_pipe_interface_sig.write(tmp_q_pipe_interface);
        }
    }
}

void Operation_combiner::detect_timestamp_match() {

    std::vector<Q_pipe_interface> vec_q_pipe_interface;
    vec_q_pipe_interface.resize(m_vliw_width);

    while (true) {
        wait();

        for (size_t i = 0; i < m_vliw_width; ++i) {
            vec_q_pipe_interface[i] = vec_in_q_pipe_interface[i].read();
            if (!reset.read() && (i_timestamp != vec_q_pipe_interface[i].timing.label)) {
                i_timestamp_match = false;
            } else {
                i_timestamp_match = true;
            }
        }
    }
}

void Operation_combiner::log_telf() {
    Q_pipe_interface q_pipe_interface;
    while (true) {

        wait();

        q_pipe_interface.reset();
        q_pipe_interface = out_q_pipe_interface.read();

        if (is_telf_on) {
            telf_os << q_pipe_interface;
        }
    }
}

void Operation_combiner::add_telf_header() {
    telf_os << std::setfill(' ') << std::setw(7) << "content"
            << " "  // instruction type
            << std::setfill(' ') << std::setw(9) << "time_type"
            << " "  // timing type
            << std::setfill(' ') << std::setw(9) << "wait_time"
            << " "  // wait time
            << std::setfill(' ') << std::setw(5) << "label"
            << " "  // timing label
            << std::setfill(' ') << std::setw(9) << "addr_type"
            << " "  // addr type
            << std::setfill(' ') << std::setw(6) << "q_type"
            << " "  // single or multi qubit operation
            << std::setfill(' ') << std::setw(10) << "reg_num"
            << " "  // register num
            << std::setfill(' ') << std::setw(10) << "mask"
            << " "  // addr mask
            << std::setfill(' ') << std::setw(24) << "qubit_addr"
            << " "  // qubit address
            << std::setfill(' ') << std::setw(8) << "op_type"
            << " "  // operation type
            << std::setfill(' ') << std::setw(10) << "op "
            << " "  // operation
            << std::endl;
}

void Operation_combiner::add_telf_line() {}

}  // end of namespace cactus
