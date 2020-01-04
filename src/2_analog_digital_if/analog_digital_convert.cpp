#include "analog_digital_convert.h"

#include "global_counter.h"

namespace cactus {

void Adi_convert::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    set_opcode_to_opname_lut(global_config.opcode_to_opname_lut);
}

void Adi_convert::set_opcode_to_opname_lut(std::map<uint64_t, std::string> lut_content) {
    opcode_to_opname_lut_content = lut_content;
}

Adi_convert::Adi_convert(const sc_core::sc_module_name& n)
    : Telf_module(n) {}

Adi_convert::~Adi_convert() {}

void Adi_convert::signal_convert() {
    // virtual function
}

// convert signal to quantumsim
Adi_convert_to_quantumsim::Adi_convert_to_quantumsim(const sc_core::sc_module_name& n)
    : Adi_convert(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    SC_CTHREAD(signal_convert, in_50MHz_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Adi_convert_to_quantumsim::~Adi_convert_to_quantumsim() {}

void Adi_convert_to_quantumsim::signal_convert() {

    auto logger        = get_logger_or_exit("console");
    auto counter_50MHz = global_counter::get("cycle_counter_50MHz");  // get cycle count

    Q_pipe_interface q_pipe_interface;
    Ops_2_qsim       i_ops;
    std::string      operation_name;
    Atom_qop         atom_qop;

    while (true) {
        wait();

        i_ops.reset();

        if (reset.read()) {  // when received reset sig
            ops_2_qsim.write(i_ops);
            continue;
        }

        q_pipe_interface = in_q_pipe_interface.read();

        for (size_t op_idx = 0; op_idx < q_pipe_interface.ops.size(); ++op_idx) {

            if (!q_pipe_interface.ops[op_idx].is_valid()) continue;

            i_ops.triggered = true;
            i_ops.cycle     = counter_50MHz->get_cur_cycle_num();

            // get operation name from lookup table
            if (q_pipe_interface.ops[op_idx].op.type == REPR_OPCODE) {
                // representation is opcode

                // check whether opcode -> opname exists
                std::map<uint64_t, std::string>::iterator it;
                it = opcode_to_opname_lut_content.find(
                  q_pipe_interface.ops[op_idx].op.opcode.get_value());
                if (it == opcode_to_opname_lut_content.end()) {
                    logger->error(
                      "{}: Opcode '0x{:x}' cannot map to a qubit gate in qubit simulator. "
                      "Simulation aborts!",
                      this->name(), q_pipe_interface.ops[op_idx].op.opcode.get_value());
                    exit(EXIT_FAILURE);
                }

                operation_name =
                  opcode_to_opname_lut_content[q_pipe_interface.ops[op_idx].op.opcode.get_value()];

            } else if (q_pipe_interface.ops[op_idx].op.type == REPR_NAME) {
                // representation is name
                operation_name = q_pipe_interface.ops[op_idx].op.name;
            } else {
                // other representation do not support
                logger->error(
                  "{}: Cannot recognize operation representation '{}' when convert operation to "
                  "the signal which qubit simulator needs. Simulation aborts!",
                  this->name(), q_pipe_interface.ops[op_idx].op.type);
                exit(EXIT_FAILURE);
            }

            atom_qop.operation = operation_name;

            // get qubit addr
            atom_qop.target_qubits.clear();  // clear before store qubit
            if (q_pipe_interface.ops[op_idx].addr.type.q_num_type == SINGLE) {  // single-qubit

                atom_qop.target_qubits.push_back(static_cast<unsigned int>(op_idx));

            } else if (q_pipe_interface.ops[op_idx].addr.type.q_num_type ==
                       MULTIPLE) {  // multi-qubit

                // left qubit
                if (op_idx == q_pipe_interface.ops[op_idx].addr.mq_op_addr.qubit_tuples[0][0]) {
                    atom_qop.target_qubits.push_back(static_cast<unsigned int>(
                      q_pipe_interface.ops[op_idx]
                        .addr.mq_op_addr.qubit_tuples[0][0]));  // multi-qubit operation,left qubit
                    atom_qop.target_qubits.push_back(static_cast<unsigned int>(
                      q_pipe_interface.ops[op_idx]
                        .addr.mq_op_addr.qubit_tuples[0][1]));  // multi-qubit operation,right qubit
                } else {
                    continue;  // right qubit
                }
            } else {
                logger->error(
                  "{}: Cannot recognize qubit gate type '{}' when convert operation to the signal "
                  "which qubit simulator needs. Simulation aborts!",
                  this->name(), q_pipe_interface.ops[op_idx].addr.type.q_num_type);
                exit(EXIT_FAILURE);
            }

            i_ops.atom_ops.push_back(atom_qop);  // write (operation,addr) to ops_2_qsim
        }

        ops_2_qsim.write(i_ops);
    }
}

// convert signal to circuit
Adi_convert_to_circuit::Adi_convert_to_circuit(const sc_core::sc_module_name& n)
    : Adi_convert(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    SC_CTHREAD(signal_convert, in_50MHz_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Adi_convert_to_circuit::~Adi_convert_to_circuit() {}

void Adi_convert_to_circuit::signal_convert() {

    auto logger        = get_logger_or_exit("console");
    auto counter_50MHz = global_counter::get("cycle_counter_50MHz");  // get cycle count

    Q_pipe_interface q_pipe_interface;
    Ops_2_qsim       i_ops;
    std::string      operation_name;
    Atom_qop         atom_qop;

    while (true) {
        wait();

        i_ops.reset();

        if (reset.read()) {  // when received reset sig
            ops_2_qsim.write(i_ops);
            continue;
        }

        q_pipe_interface = in_q_pipe_interface.read();

        // iterate each qubit
        for (size_t op_idx = 0; op_idx < q_pipe_interface.ops.size(); ++op_idx) {

            if (!q_pipe_interface.ops[op_idx].is_valid()) continue;

            i_ops.triggered = true;
            i_ops.cycle     = counter_50MHz->get_cur_cycle_num();

            // get operation name from lookup table
            if (q_pipe_interface.ops[op_idx].op.type == REPR_OPCODE) {
                // representation is opcode

                // check whether opcode -> opname exists
                std::map<uint64_t, std::string>::iterator it;
                it = opcode_to_opname_lut_content.find(
                  q_pipe_interface.ops[op_idx].op.opcode.get_value());
                if (it == opcode_to_opname_lut_content.end()) {
                    logger->error(
                      "{}: Opcode '0x{:x}' cannot map to a qubit gate in qubit simulator. "
                      "Simulation aborts!",
                      this->name(), q_pipe_interface.ops[op_idx].op.opcode.get_value());
                    exit(EXIT_FAILURE);
                }

                operation_name =
                  opcode_to_opname_lut_content[q_pipe_interface.ops[op_idx].op.opcode.get_value()];

            } else if (q_pipe_interface.ops[op_idx].op.type == REPR_NAME) {
                // representation is name
                operation_name = q_pipe_interface.ops[op_idx].op.name;
            } else {
                // other representation do not support
                logger->error(
                  "{}: Cannot recognize operation representation '{}' when convert operation to "
                  "the signal which qubit simulator needs. Simulation aborts!",
                  this->name(), q_pipe_interface.ops[op_idx].op.type);
                exit(EXIT_FAILURE);
            }

            atom_qop.operation = operation_name;

            // get qubit addr
            atom_qop.target_qubits.clear();  // clear before store qubit
            if (q_pipe_interface.ops[op_idx].addr.type.q_num_type == SINGLE) {  // single-qubit

                atom_qop.target_qubits.push_back(static_cast<unsigned int>(op_idx));

            } else if (q_pipe_interface.ops[op_idx].addr.type.q_num_type ==
                       MULTIPLE) {  // multi-qubit

                // left qubit
                if (op_idx == q_pipe_interface.ops[op_idx].addr.mq_op_addr.qubit_tuples[0][0]) {
                    atom_qop.target_qubits.push_back(static_cast<unsigned int>(
                      q_pipe_interface.ops[op_idx]
                        .addr.mq_op_addr.qubit_tuples[0][0]));  // multi-qubit operation,left qubit
                    atom_qop.target_qubits.push_back(static_cast<unsigned int>(
                      q_pipe_interface.ops[op_idx]
                        .addr.mq_op_addr.qubit_tuples[0][1]));  // multi-qubit operation,right qubit
                } else {
                    continue;  // right qubit
                }
            } else {
                logger->error(
                  "{}: Cannot recognize qubit gate type '{}' when convert operation to the signal "
                  "which qubit simulator needs. Simulation aborts!",
                  this->name(), q_pipe_interface.ops[op_idx].addr.type.q_num_type);
                exit(EXIT_FAILURE);
            }

            i_ops.atom_ops.push_back(atom_qop);  // write (operation,addr) to ops_2_qsim
        }

        ops_2_qsim.write(i_ops);
    }
}

}  // end of namespace cactus
