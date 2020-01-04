#include "q_decoder_bin.h"

namespace cactus {

void Q_decoder_bin::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
    m_vliw_width = global_config.vliw_width;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "q_decoder_bin");
}

Q_decoder_bin::Q_decoder_bin(const sc_core::sc_module_name& n)
    : Q_decoder(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    open_telf_file();

    SC_CTHREAD(do_output, in_clock.pos());

    SC_CTHREAD(log_telf, in_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Q_decoder_bin::~Q_decoder_bin() { close_telf_file(); }

void Q_decoder_bin::do_output() {

    auto logger = get_logger_or_exit("telf_logger");

    sc_uint<QISA_WIDTH>     bundle;     // 32bit instruction
    sc_uint<M_OPCODE_WIDTH> m_op_code;  // multiple operation format
    sc_uint<S_OPCODE_WIDTH> s_op_code;  // single operation format

    Qasm_instruction cur_insn;
    Q_pipe_interface q_pipe_interface;
    q_pipe_interface.vliw_width = m_vliw_width;

    bool is_valid_wait     = false;
    bool is_valid_qop      = false;
    bool is_valid_set_addr = false;
    bool wr_s_or_t         = false;

    unsigned int op_wait_time;
    Bare_qop     qop;
    Sim_uint     mask;
    Sim_uint     reg_num;
    Q_tgt_addr   addr_to_set;

    std::vector<Fledged_qop> q_op;
    q_op.resize(m_vliw_width);

    while (true) {
        wait();

        // clear everything at the begin of every cycle
        q_pipe_interface.reset();

        if (reset.read()) {  // when receive reset signal
            q_pipe_interface.if_content.valid_qop      = false;
            q_pipe_interface.if_content.valid_set_addr = false;
            q_pipe_interface.if_content.valid_wait     = false;

            for (size_t i = 0; i < q_pipe_interface.vliw_width; ++i) {
                q_op[i].reset();
                q_pipe_interface.ops.push_back(q_op[i]);
            }

            // do output
            out_q_pipe_interface.write(q_pipe_interface);

            continue;
        }

        is_valid_wait     = false;  // wait instruction
        is_valid_qop      = false;  // vliw instruction
        is_valid_set_addr = false;  // address operation instruction
        op_wait_time      = 0;

        if (in_valid_bundle.read()) {

            cur_insn = in_bundle.read();
            bundle   = cur_insn.get_insn_bin();

            if (bundle[QISA_WIDTH - 1]) {  // MSb==1: vliw instruction

                is_valid_qop  = true;
                is_valid_wait = true;
                op_wait_time  = bundle.range(2, 0).to_uint();

            } else {  // MSb==0: single format instruction

                s_op_code = bundle.range(30, 25);

                // opcodes of SMIS, SMIT, QWAIT, QWAITR
                // QWAITR: 0b111000
                // QWAIT:  0b110000
                // SMIS:   0b100000
                // SMIT:   0b101000
                if (s_op_code.range(5, 4) == 0b11) {
                    is_valid_wait = true;

                    // waiting time
                    if (s_op_code[3] == 1) {  // QWAITR
                        op_wait_time = in_rs_wait_time.read().to_uint();
                    } else {  // QWAIT
                        op_wait_time = bundle.range(19, 0).to_uint();
                    }
                } else if (s_op_code.range(5, 4) == 0b10) {
                    is_valid_set_addr = true;

                    // single-qubit operation or multi-qubit operation
                    wr_s_or_t = s_op_code[3] ? false : true;

                    // [24:20] register number
                    reg_num = bundle.range(24, 20).to_uint();
                }
            }
        }

        // set timing info
        if (is_valid_wait && (op_wait_time > 0)) {
            q_pipe_interface.if_content.valid_wait = true;
            q_pipe_interface.timing.type           = WAIT_TIME;
            q_pipe_interface.timing.wait_time      = op_wait_time;

            logger->debug("{}: content_type:wait,wait_time:0x{:x}", this->name(),
                          q_pipe_interface.timing.wait_time);

        } else {
            q_pipe_interface.if_content.valid_wait = false;
        }

        q_pipe_interface.if_content.valid_qop      = is_valid_qop;
        q_pipe_interface.if_content.valid_set_addr = is_valid_set_addr;

        // set addr info
        if (is_valid_set_addr) {
            // addressing type
            q_pipe_interface.type.c_type     = INDIRECT_REG_NUM;
            q_pipe_interface.type.q_num_type = wr_s_or_t ? SINGLE : MULTIPLE;

            addr_to_set.reset();
            addr_to_set.type                  = q_pipe_interface.type;
            addr_to_set.indirect_addr_reg_num = reg_num;

            // set addr mask
            if (wr_s_or_t) {  // SMIS
                addr_to_set.sq_op_addr.mask       = bundle.range(6, 0).to_uint();
                addr_to_set.sq_op_addr.somq_width = 7;  // max 7 single qubits

                logger->debug("{}: content_type:addr,mask:0x{:x}", this->name(),
                              addr_to_set.sq_op_addr.mask.get_value());
            } else {  // SMIT
                addr_to_set.mq_op_addr.mask       = bundle.range(15, 0).to_uint();
                addr_to_set.mq_op_addr.somq_width = 16;  // max 16 qubit tuples

                logger->debug("{}: content_type:addr,mask:0x{:x}", this->name(),
                              addr_to_set.mq_op_addr.mask.get_value());
            }

            q_pipe_interface.addrs_to_set.push_back(addr_to_set);
        }

        // set vliw operation info
        // [30:23] opcode of quantum operation 0
        // [22:17] register num of quantum operation 0
        // [16:9] opcode of quantum operation 1
        // [8:3] register num of quantum operation 1
        // [2:0] wait time
        for (size_t i = 0; i < q_pipe_interface.vliw_width; ++i) {
            q_op[i].reset();

            if (is_valid_qop && i < 2) {  // 32bit instruction only support 2 vliw pipelanes

                m_op_code = (i == 0) ? bundle.range(30, 23) : bundle.range(16, 9);

                // if not NOP operation
                if (m_op_code) {  // 0 : represents NOP operation

                    q_op[i].timing = q_pipe_interface.timing;  // timing info

                    q_op[i].op.type   = REPR_OPCODE;  // set operation represent format
                    q_op[i].op.opcode = m_op_code.to_uint();

                    q_op[i].addr.type.c_type = INDIRECT_REG_NUM;  // set addressing type
                    q_op[i].addr.type.q_num_type =
                      m_op_code[M_OPCODE_WIDTH - 1] ? MULTIPLE : SINGLE;

                    q_op[i].addr.indirect_addr_reg_num =
                      (i == 0) ? bundle.range(22, 17).to_uint()
                               : bundle.range(8, 3).to_uint();  // set register number

                    logger->debug("{}: content_type:qop,reg_num:{},opcode:0x{:x} .", this->name(),
                                  q_op[i].addr.indirect_addr_reg_num.get_value(),
                                  q_op[i].op.opcode.get_value());
                }
            }
            q_pipe_interface.ops.push_back(q_op[i]);
        }

        // do output
        out_q_pipe_interface.write(q_pipe_interface);
    }
}

void Q_decoder_bin::log_telf() {
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

void Q_decoder_bin::add_telf_header() {
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

void Q_decoder_bin::add_telf_line() {
    // TODO: add telf_line of q_decoder
    // this function is replaced by override operator << , which called in thread log_telf
}

}  // namespace cactus
