#include "classical_execute.h"

#include <sstream>

#include "num_util.h"

namespace cactus {

void Classical_execute::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "classical_execute");
};

Classical_execute::Classical_execute(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();
    open_telf_file();

    reg_file.init(REG_FILE_NUM);
    flags_cmp.init(16);
    flags_test.init(11);

    // execute stage
    SC_CTHREAD(de2ex_ff, clock.pos());
    SC_THREAD(write_to_qp);
    sensitive << ex_insn << ex_q_valid << ex_rs_addr;

    SC_THREAD(execution);
    sensitive << ex_opcode << ex_insn << ex_run << ex_pc;

    SC_CTHREAD(clock_counter, clock.pos());

    // log method
    if (is_telf_on) {
        SC_CTHREAD(write_insn_file, clock.pos());
    }

    logger->trace("Finished initializing {}...", this->name());
}

Classical_execute::~Classical_execute() { close_telf_file(); }

void Classical_execute::de2ex_ff() {
    while (true) {
        wait();

        // initiated at the beginning
        flags_cmp[0] = 1;  // 0: always
        flags_cmp[1] = 0;  // 1: never

        ex_insn.write(de_insn.read());

        // the instruction sent to quantum pipeline is invalid when the pipeline is stalled
        if (de_stall.read()) {
            ex_q_valid.write(0);
            ex_meas_ena.write(0);
            ex_run.write(0);
        } else {
            ex_q_valid.write(de_q_valid.read());
            ex_meas_ena.write(de_meas_ena.read());
            ex_run.write(de_run.read());
        }

        de_fmr_ready.write(de_fmr_ready_next.read());
        ex_qmr_data.write(de_qmr_data.read());

        ex_pc.write(de_pc.read());
        // out_ex_run.write(de_run.read());
        ex_opcode.write(de_opcode.read());
        ex_rs_addr.write(de_rs_addr.read());
        ex_rt_addr.write(de_rt_addr.read());
        ex_rd_addr.write(de_rd_addr.read());
        ex_imm.write(de_imm.read());
        // ex_imm_sign.write(de_imm_sign.read());
        ex_uimm.write(de_uimm.read());
        ex_br_addr.write(de_br_addr.read());
        // ex_br_addr_sign.write(de_br_addr_sign.read());
        ex_br_cond.write(de_br_cond.read());
        ex_insn_use_rd.write(de_insn_use_rd.read());
    }
}

void Classical_execute::write_to_qp() {
    sc_int<INSN_WIDTH> Rs_v;
    while (true) {
        wait();

        Clp2Qp_insn  = ex_insn.read();
        Clp2Qp_valid = ex_q_valid.read();
        Rs_v         = reg_file[static_cast<size_t>(ex_rs_addr.read())];
        Clp2Qp_Rs.write(unsigned(Rs_v));
    }
}

void Classical_execute::execution() {
    sc_int<INSN_WIDTH> Rs_v;
    sc_int<INSN_WIDTH> Rt_v;
    sc_int<INSN_WIDTH> Rd_v;

    Qasm_instruction cur_insn;

    bool            wr_rd_en     = false;
    bool            mem_strobe   = false;
    bool            mem_rw       = false;
    MEM_ACCESS_TYPE mem_addr_sel = ADDR_WORD;
    bool            mem_sext     = false;
    bool            ex2reg       = true;

    auto logger = get_logger_or_exit("console");

    while (true) {
        wait();

        if (ex_run) {
            Rs_v           = reg_file[static_cast<size_t>(ex_rs_addr.read())];
            Rt_v           = reg_file[static_cast<size_t>(ex_rt_addr.read())];
            Rd_v           = 0;
            wr_rd_en       = false;
            ex_cond_result = false;
            ex2reg         = true;
            mem_strobe     = false;
            mem_rw         = false;
            mem_addr_sel   = ADDR_WORD;
            mem_sext       = false;

            // Operand forwarding. If the previous instruction needs to write a value back to a
            // register, and this value is used by current instruction, then this value is read
            // into current operand forwarding T operand
            // forwarding T operand
            if ((ex_rt_addr.read() == mem_rd_addr.read()) && mem_run.read() &&
                mem_wr_rd_en.read()) {
                Rt_v = mem_ex_rd_value.read();  // mem stage
            } else if ((ex_rt_addr.read() == wb_rd_addr.read()) && wb_run.read() &&
                       wb_wr_rd_en.read()) {
                Rt_v = wb_rd_value.read();  // wb stage
            }
            // forwarding S operand
            if ((ex_rs_addr.read() == mem_rd_addr.read()) && mem_run.read() &&
                mem_wr_rd_en.read()) {
                Rs_v = mem_ex_rd_value.read();  // mem stage
            } else if ((ex_rs_addr.read() == wb_rd_addr.read()) && wb_run.read() &&
                       wb_wr_rd_en.read()) {
                Rs_v = wb_rd_value.read();  // wb stage
            }

            switch (ex_opcode.read()) {
                case OperationName::NOP:
                    break;
                case OperationName::ADD:
                    Rd_v     = Rs_v + Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::ADDI:
                    Rd_v     = Rs_v + ex_imm.read();
                    wr_rd_en = true;
                    break;
                case OperationName::SUB:
                    Rd_v     = Rs_v - Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::DIV:
                    if (Rt_v == 0) {
                        logger->error("{}: Integer division by zero. Simulation aborts!",
                                      this->name());
                        exit(EXIT_FAILURE);
                    }
                    Rd_v     = Rs_v / Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::MUL:
                    Rd_v     = Rs_v * Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::REM:
                    Rd_v     = Rs_v % Rt_v;
                    wr_rd_en = true;
                    break;

                case OperationName::OR:
                    Rd_v     = Rs_v | Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::STOP:
                    de_done.write(true);
                    break;
                case OperationName::AND:
                    Rd_v     = Rs_v & Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::XOR:
                    Rd_v     = Rs_v ^ Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::NOT:
                    Rd_v     = ~Rt_v;
                    wr_rd_en = true;
                    break;
                case OperationName::LDI:
                    Rd_v = ex_imm.read();
                    // if (sign_imm) {                                          //if imm is a
                    // negative number, sign extension for bit31 - bit20 is required
                    //    Rd_v = Rd_v | -1048576;
                    //}
                    wr_rd_en = true;
                    break;
                case OperationName::LDUI:
                    Rd_v.range(31, 17) = ex_uimm.read();
                    Rd_v.range(16, 0)  = Rs_v.range(16, 0);
                    // Rd_v = (uimm << 17) + (Rs_v & 0x1FFFF);                  //Rd_v = uimm[14..0]
                    // :: Rs_v[16..0]
                    wr_rd_en = true;
                    break;
                case OperationName::CMP:
                    // The never and always conditions are already initialized at the beginning of
                    // simulation flags_cmp[0] = 1;                                            // 0:
                    // always flags_cmp[1] = 0;                                            // 1:
                    // never
                    flags_cmp[2]  = (Rs_v == Rt_v);                              // 2: eq
                    flags_cmp[3]  = !(Rs_v == Rt_v);                             // 3: ne
                    flags_cmp[8]  = (unsigned int) Rs_v < (unsigned int) Rt_v;   // 8: ltu
                    flags_cmp[9]  = (unsigned int) Rs_v >= (unsigned int) Rt_v;  // 9: geu
                    flags_cmp[10] = (unsigned int) Rs_v <= (unsigned int) Rt_v;  // 10: leu
                    flags_cmp[11] = (unsigned int) Rs_v > (unsigned int) Rt_v;   // 11: gtu
                    flags_cmp[12] = Rs_v < Rt_v;                                 // 12: lt
                    flags_cmp[13] = Rs_v >= Rt_v;                                // 13: ge
                    flags_cmp[14] = Rs_v <= Rt_v;                                // 14: le
                    flags_cmp[15] = Rs_v > Rt_v;                                 // 15: gt
                    break;
                case OperationName::TEST:
                    flags_test[0] = 1;                                          // 0: always
                    flags_test[1] = 0;                                          // 1: never
                    flags_test[5] = (Rs_v < 0);                                 // 5: ltz
                    flags_test[6] = !(Rs_v >= 0);                               // 6: gez
                    flags_test[7] = (unsigned int) Rs_v < (unsigned int) Rt_v;  // 7: notcarry   !!!
                    flags_test[8] = (unsigned int) Rs_v >= (unsigned int) Rt_v;  // 8: carry !!!
                    flags_test[9] = Rs_v == 0;                                   // 9: eqz
                    flags_test[10] = Rs_v != 0;                                  // 10: nez
                    break;
                case OperationName::ADDC:
                    Rd_v           = Rs_v + Rt_v;
                    wr_rd_en       = true;
                    flags_test[0]  = 1;  // ADDC instruction will set branch register like TEST
                    flags_test[1]  = 0;
                    flags_test[5]  = (Rs_v < 0);
                    flags_test[6]  = !(Rs_v >= 0);
                    flags_test[7]  = (unsigned int) Rs_v < (unsigned int) Rt_v;
                    flags_test[8]  = (unsigned int) Rs_v >= (unsigned int) Rt_v;
                    flags_test[9]  = Rs_v == 0;
                    flags_test[10] = Rs_v != 0;
                    break;
                case OperationName::SUBC:
                    Rd_v          = Rs_v + Rt_v;
                    wr_rd_en      = true;
                    flags_cmp[0]  = 1;  // SUBC instruction will set branch register like CMP
                    flags_cmp[1]  = 0;
                    flags_cmp[2]  = (Rs_v == Rt_v);
                    flags_cmp[3]  = !(Rs_v == Rt_v);
                    flags_cmp[8]  = (unsigned int) Rs_v < (unsigned int) Rt_v;
                    flags_cmp[9]  = (unsigned int) Rs_v >= (unsigned int) Rt_v;
                    flags_cmp[10] = (unsigned int) Rs_v <= (unsigned int) Rt_v;
                    flags_cmp[11] = (unsigned int) Rs_v > (unsigned int) Rt_v;
                    flags_cmp[12] = Rs_v < Rt_v;
                    flags_cmp[13] = Rs_v >= Rt_v;
                    flags_cmp[14] = Rs_v <= Rt_v;
                    flags_cmp[15] = Rs_v > Rt_v;
                    break;
                case OperationName::FBR:
                    Rd_v     = flags_cmp[static_cast<size_t>(ex_br_cond.read())].read();
                    wr_rd_en = true;
                    break;
                case OperationName::FMR:
                    Rd_v     = ex_qmr_data.read();
                    wr_rd_en = true;
                    break;
                case OperationName::BR:
                    ex_cond_result.write(flags_cmp[static_cast<size_t>(ex_br_cond.read())].read());
                    break;
                case OperationName::LB:
                    Rd_v         = Rs_v + ex_imm.read();
                    wr_rd_en     = true;
                    ex2reg       = false;
                    mem_strobe   = true;
                    mem_rw       = true;
                    mem_addr_sel = ADDR_BYTE;
                    mem_sext     = true;
                    break;
                case OperationName::LBU:
                    Rd_v         = Rs_v + ex_imm.read();
                    wr_rd_en     = true;
                    ex2reg       = false;
                    mem_strobe   = true;
                    mem_rw       = true;
                    mem_addr_sel = ADDR_BYTE;
                    mem_sext     = false;
                    break;
                case OperationName::LW:
                    Rd_v         = Rs_v + ex_imm.read();
                    wr_rd_en     = true;
                    ex2reg       = false;
                    mem_strobe   = true;
                    mem_rw       = true;
                    mem_addr_sel = ADDR_WORD;
                    mem_sext     = false;
                    break;
                case OperationName::SB:
                    Rd_v         = Rs_v + ex_imm.read();
                    wr_rd_en     = false;
                    ex2reg       = false;
                    mem_strobe   = true;
                    mem_rw       = false;
                    mem_addr_sel = ADDR_BYTE;
                    mem_sext     = false;
                    break;
                case OperationName::SW:
                    Rd_v         = Rs_v + ex_imm.read();
                    wr_rd_en     = false;
                    ex2reg       = false;
                    mem_strobe   = true;
                    mem_rw       = false;
                    mem_addr_sel = ADDR_WORD;
                    mem_sext     = false;
                    break;
                default:
                    if (!ex_q_valid.read()) {
                        cur_insn = de_insn.read();
                        if (cur_insn.get_type() == Instruction_type::BIN) {
                            logger->error(
                              "{}: Unrecognized instruction '{:08x}'. Simulation aborts!",
                              this->name(), cur_insn.get_insn_bin());
                        } else {
                            logger->error("{}: Unrecognized instruction '{}'. Simulation aborts!",
                                          this->name(), cur_insn.get_insn_asm());
                        }

                        exit(EXIT_FAILURE);
                    }
            }

            ex_rd_value.write(Rd_v);
            ex_wr_rd_en.write(wr_rd_en);
            ex_mem_rw.write(mem_rw);
            ex_mem_strobe.write(mem_strobe);
            ex_mem_data.write(Rt_v);
            ex_mem_addr_sel.write(mem_addr_sel);
            ex_mem_sext.write(mem_sext);
            ex_ex2reg.write(ex2reg);
        }
    }
}

void Classical_execute::write_insn_file() {
    Qasm_instruction cur_insn;

    while (true) {
        wait();

        if (de_run.read() && !de_stall.read()) {
            cur_insn = de_insn.read();
            telf_os << std::setfill(' ') << std::setw(15) << sc_core::sc_time_stamp().to_string();
            telf_os << ",    " << std::setfill(' ') << std::setw(11) << std::dec << m_num_cycles;
            telf_os << ",    " << std::setfill(' ') << std::setw(6) << de_pc;
            if (cur_insn.get_type() == Instruction_type::BIN) {
                telf_os << ",    " << std::setfill(' ') << std::setw(25)
                        << int_2_hex_str(cur_insn.get_insn_bin());
            } else {
                telf_os << ",    " << std::setfill(' ') << std::setw(25) << cur_insn.get_insn_asm();
            }
            telf_os << std::endl;
        }
    }
}

void Classical_execute::clock_counter() {
    while (true) {
        wait();
        m_num_cycles++;
    }
}

void Classical_execute::add_telf_header() {
    // Specify the signal type
    telf_os << "#Insn" << std::endl;

    telf_os << "       sim time,    Clock cycle,      addr,                         insn"
            << std::endl;
}

}  // namespace cactus
