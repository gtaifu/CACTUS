#include "q_decoder_asm.h"

#include <algorithm>
#include <regex>  // regular expression

#include "global_json.h"

namespace cactus {

void Q_decoder_asm::config() {
    Global_config& global_config = Global_config::get_instance();

    m_vliw_width = global_config.vliw_width;
    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "q_decoder_asm");
}

Q_decoder_asm::Q_decoder_asm(const sc_core::sc_module_name& n)
    : Q_decoder(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    open_telf_file();

    SC_CTHREAD(output, in_clock.pos());

    SC_CTHREAD(log_telf, in_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Q_decoder_asm::~Q_decoder_asm() { close_telf_file(); }

void Q_decoder_asm::output() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface q_pipe_interface;
    Qasm_instruction cur_insn;

    unsigned int rs_wait;

    while (true) {
        wait();

        q_pipe_interface.reset();  // clear everything at the beginning of each clock

        if (reset.read()) {  // when recieve reset sig

            set_nop(q_pipe_interface);
            out_q_pipe_interface.write(q_pipe_interface);
            continue;
        }

        cur_insn = in_bundle.read();
        rs_wait  = in_rs_wait_time.read().to_uint();

        if (in_valid_bundle.read()) {
            cur_insn.parse_q_insn();
            try {
                set_q_insn(q_pipe_interface, cur_insn, rs_wait);
            } catch (...) {
                logger->error(
                  "{}: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
                  this->name(), cur_insn.get_insn_str_in_file(),
                  cur_insn.get_insn_line_num_in_file());
                exit(EXIT_FAILURE);
            }
        } else {
            set_nop(q_pipe_interface);
        }

        out_q_pipe_interface.write(q_pipe_interface);
    }
}

void Q_decoder_asm::set_q_insn(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction,
                               const unsigned int& rs_wait) {

    switch (instruction.get_q_insn_type()) {
        case Q_instr_type::Q_OP:
            set_qop(q_pipe_interface, instruction);
            break;
        case Q_instr_type::Q_WAIT:
        case Q_instr_type::Q_WAITR:
            set_wait(q_pipe_interface, instruction, rs_wait);
            break;
        case Q_instr_type::Q_SMIS:
            set_smis(q_pipe_interface, instruction);
            break;
        case Q_instr_type::Q_SMIT:
            set_smit(q_pipe_interface, instruction);
            break;

        default:
            set_nop(q_pipe_interface);
            break;
    }
}

void Q_decoder_asm::set_smis(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction) {

    Q_tgt_addr addr_to_set;

    q_pipe_interface.if_content.valid_wait     = false;
    q_pipe_interface.if_content.valid_qop      = false;
    q_pipe_interface.if_content.valid_set_addr = true;

    // set addr info
    q_pipe_interface.type.c_type     = INDIRECT_REG_CONTENT;
    q_pipe_interface.type.q_num_type = SINGLE;

    addr_to_set.type                  = q_pipe_interface.type;
    addr_to_set.indirect_addr_reg_num = instruction.get_q_reg_num()[0];
    addr_to_set.sq_op_addr.qubit_indices.assign(instruction.get_q_qubit_indices().begin(),
                                                instruction.get_q_qubit_indices().end());
    addr_to_set.sq_op_addr.somq_width = m_num_qubits;

    q_pipe_interface.addrs_to_set.push_back(addr_to_set);

    for (size_t i = 0; i < m_vliw_width; ++i) {  // reset operations
        Fledged_qop fledge_qop;
        fledge_qop.reset();
        q_pipe_interface.ops.push_back(fledge_qop);
    }
}

void Q_decoder_asm::set_smit(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction) {

    Q_tgt_addr addr_to_set;
    q_pipe_interface.if_content.valid_wait     = false;
    q_pipe_interface.if_content.valid_qop      = false;
    q_pipe_interface.if_content.valid_set_addr = true;

    // set addr info
    q_pipe_interface.type.c_type     = INDIRECT_REG_CONTENT;
    q_pipe_interface.type.q_num_type = MULTIPLE;

    addr_to_set.type                  = q_pipe_interface.type;
    addr_to_set.indirect_addr_reg_num = instruction.get_q_reg_num()[0];
    addr_to_set.mq_op_addr.qubit_tuples.assign(instruction.get_q_qubit_tuples().begin(),
                                               instruction.get_q_qubit_tuples().end());
    addr_to_set.mq_op_addr.somq_width = m_num_qubits;

    q_pipe_interface.addrs_to_set.push_back(addr_to_set);

    for (size_t i = 0; i < m_vliw_width; ++i) {  // reset operations
        Fledged_qop fledge_qop;
        fledge_qop.reset();
        q_pipe_interface.ops.push_back(fledge_qop);
    }
}

void Q_decoder_asm::set_wait(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction,
                             const unsigned int& rs_wait) {
    auto logger = get_logger_or_exit("telf_logger");

    unsigned int wait_time;
    if (instruction.get_q_insn_type() == Q_instr_type::Q_WAITR) {
        wait_time = rs_wait;
    } else {
        wait_time = instruction.get_q_time_specified();
    }

    // set content type
    q_pipe_interface.if_content.valid_qop      = false;
    q_pipe_interface.if_content.valid_set_addr = false;

    if (wait_time > 0) {
        q_pipe_interface.if_content.valid_wait = true;
        q_pipe_interface.timing.type           = WAIT_TIME;
        q_pipe_interface.timing.wait_time      = wait_time;
    } else {
        q_pipe_interface.if_content.valid_wait = false;
    }

    logger->trace("{}: content_type:wait, wait_time:0x{:x}", this->name(), wait_time);

    for (size_t i = 0; i < m_vliw_width; ++i) {  // reset operations
        Fledged_qop fledge_qop;
        fledge_qop.reset();
        q_pipe_interface.ops.push_back(fledge_qop);
    }
}

void Q_decoder_asm::set_qop(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction) {

    auto logger = get_logger_or_exit("telf_logger");

    unsigned int wait_time = instruction.get_q_time_specified();

    q_pipe_interface.if_content.valid_qop      = true;
    q_pipe_interface.if_content.valid_set_addr = false;

    // set timing info
    if (wait_time > 0) {
        q_pipe_interface.if_content.valid_wait = true;
        q_pipe_interface.timing.type           = WAIT_TIME;
        q_pipe_interface.timing.wait_time      = wait_time;
    } else {
        q_pipe_interface.if_content.valid_wait = false;
    }

    // check vliw_width
    if (instruction.get_q_op_name().size() > m_vliw_width) {
        logger->error(
          "{}: Instruction '{}' at line {} has {} operations, exceeds VLIW width {}. Simulation "
          "aborts!",
          this->name(), instruction.get_insn_str_in_file(), instruction.get_insn_line_num_in_file(),
          instruction.get_q_op_name().size(), m_vliw_width);
        exit(EXIT_FAILURE);
    }

    // set operation info
    for (size_t i = 0; i < instruction.get_q_op_name().size(); ++i) {
        Fledged_qop fledge_qop;

        fledge_qop.timing  = q_pipe_interface.timing;
        fledge_qop.op.type = REPR_NAME;
        fledge_qop.op.name = instruction.get_q_op_name()[i];

        fledge_qop.addr.type.c_type = INDIRECT_REG_CONTENT;
        // single qubit operation or multi qubit operation
        fledge_qop.addr.type.q_num_type       = instruction.get_q_num_tgt_qubits_type()[i];
        fledge_qop.addr.indirect_addr_reg_num = instruction.get_q_reg_num()[i];

        q_pipe_interface.ops.push_back(fledge_qop);
    }
    for (size_t i = instruction.get_q_op_name().size(); i < m_vliw_width; ++i) {
        Fledged_qop fledge_qop;
        fledge_qop.reset();
        q_pipe_interface.ops.push_back(fledge_qop);
    }
}

void Q_decoder_asm::set_nop(Q_pipe_interface& q_pipe_interface) {
    q_pipe_interface.if_content.valid_wait     = false;
    q_pipe_interface.if_content.valid_qop      = false;
    q_pipe_interface.if_content.valid_set_addr = false;

    for (size_t i = 0; i < m_vliw_width; ++i) {  // reset operations
        Fledged_qop fledge_qop;
        fledge_qop.reset();
        q_pipe_interface.ops.push_back(fledge_qop);
    }
}

void Q_decoder_asm::log_telf() {
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

void Q_decoder_asm::add_telf_header() {
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

void Q_decoder_asm::add_telf_line() {
    // TODO: add telf_line of q_decoder
    // this function is replaced by override operator << , which called in thread log_telf
}

}  // namespace cactus
