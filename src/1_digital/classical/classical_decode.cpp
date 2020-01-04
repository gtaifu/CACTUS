#include "classical_decode.h"

#include <sstream>

#include "num_util.h"

namespace cactus {

void Classical_decode::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "classical_decode");
};

Classical_decode::Classical_decode(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();
    open_telf_file();

    MRF2Clp_data.init(m_num_qubits);
    MRF2Clp_valid.init(m_num_qubits);
    de_qmr_data_all.init(m_num_qubits);
    de_qmr_valid_all.init(m_num_qubits);
    flags_cmp_dly.init(16);

    // decode stage
    SC_THREAD(mrf_in);
    sensitive << init;
    for (size_t i = 0; i < m_num_qubits; ++i) {
        sensitive << MRF2Clp_data[i] << MRF2Clp_valid[i];
    }

    SC_CTHREAD(if2de_ff, clock.pos());
    SC_THREAD(write_output);
    sensitive << de_done << de_clk_en << de_br_start << if_target_pc << de_pc;

    SC_THREAD(decode_stage);
    sensitive << de_insn << de_insn_valid << de_run;

    SC_THREAD(br_start);
    sensitive << de_insn << de_reset << de_reset_dly << init;

    SC_THREAD(stalling_logic);
    sensitive << de_qp_ready << de_cl_valid << de_is_fmr << de_qmr_ready << de_fmr_ready_lock
              << load_use_hazard << de_done;

    // detect load-use hazard
    SC_THREAD(hazard_detection);
    sensitive << de_run << ex_run << ex_opcode << ex_rd_addr << de_rs_addr << de_rt_addr
              << de_insn_use_rs << de_insn_use_rt;

    SC_THREAD(measurement_valid_detection);
    sensitive << de_insn;
    for (size_t i = 0; i < m_num_qubits; ++i) {
        sensitive << de_qmr_valid_all[i];
    }

    SC_THREAD(measurement_result_selection);
    sensitive << de_insn;
    for (size_t i = 0; i < m_num_qubits; ++i) {
        sensitive << de_qmr_data_all[i];
    }

    SC_THREAD(fmr_ready_logic);
    sensitive << de_clk_en << de_cl_valid << de_is_fmr << de_qmr_all_valid << de_qmr_sel_valid
              << ex_meas_ena;

    SC_THREAD(fmr_interlocking_logic);
    sensitive << de_fmr_ready << ex_meas_ena;

    SC_CTHREAD(clock_counter, clock.pos());

    if (is_telf_on) {
        SC_CTHREAD(write_insn_file, clock.pos());
    }

    logger->trace("Finished initializing {}...", this->name());
}

Classical_decode::~Classical_decode() { close_telf_file(); }

void Classical_decode::if2de_ff() {
    while (true) {
        wait();

        de_init_pc.write(if_init_pc.read());
        de_reset.write(if_reset.read());
        // de_done.write(if_done.read()); // modified by Xiang.
        de_reset_dly.write(if_reset_dly.read());

        if (de_stall.read())
            de_run = de_run.read();
        else
            de_run.write(if_run.read());  // run signal goes low when the pipeline is stalled

        // out_de_run.write(if_run.read());

        if (de_clk_en.read()) {
            if (de_br_start.read())
                de_pc.write(if_target_pc.read());
            else
                de_pc.write(if_normal_pc.read());
        }
    }
}

void Classical_decode::mrf_in() {
    while (true) {
        wait();

        //**************NOTE********************
        // instruction overflow never happens in current design
        // de_qmr_ready.write(MRF2Clp_ready.read());
        de_qmr_ready.write(true);
        if (init.read()) {
            for (size_t i = 0; i < m_num_qubits; ++i) {
                de_qmr_data_all[i].write(0);
                de_qmr_valid_all[i].write(1);
            }
        } else {
            for (size_t i = 0; i < m_num_qubits; ++i) {
                de_qmr_data_all[i].write(MRF2Clp_data[i].read());
                de_qmr_valid_all[i].write(MRF2Clp_valid[i].read());
            }
        }
    }
}

void Classical_decode::br_start() {
    Qasm_instruction insn;
    sc_uint<32>      cond;
    bool             is_br;
    bool             cond_result;
    while (true) {
        wait();

        if (de_insn_valid) insn = de_insn.read();

        is_br       = insn.is_br();
        cond        = insn.get_br_cond();
        cond_result = flags_cmp_dly[static_cast<size_t>(cond)];

        if (is_br && de_run.read())
            de_br_start.write(cond_result);
        else if (de_reset_dly.read() && !de_reset.read())
            // First cycle after reset. Here we must force a branch to PCInit
            // to get everything going
            de_br_start.write(true);
        else
            de_br_start.write(false);
    }
}

void Classical_decode::fmr_interlocking_logic() {
    while (true) {
        wait();

        de_fmr_ready_lock.write(de_fmr_ready.read() & !ex_meas_ena.read());
    }
}

void Classical_decode::stalling_logic() {
    bool v_clk_en;
    while (true) {
        wait();

        v_clk_en = 1;

        // stall when quantum measurement registers are not ready
        if (!de_qmr_ready.read()) v_clk_en = 0;

        // stall when quantum pipeline is not ready to receive instructions
        if (!de_qp_ready.read()) {
            v_clk_en = 0;
        }

        // load-use hazard is detected
        if (load_use_hazard.read()) {
            v_clk_en = 0;
        }

        // received stop instruction
        if (de_done.read()) {
            v_clk_en = 0;
        }

        if (de_cl_valid.read() && de_is_fmr.read()) {
            if (!de_fmr_ready_lock.read()) {
                v_clk_en = 0;
            }
        }

        de_clk_en.write(v_clk_en);
        de_stall.write(!v_clk_en);
    }
}

void Classical_decode::hazard_detection() {
    while (true) {
        wait();

        load_use_hazard.write(false);

        // detect  load-use hazard
        if (ex_run.read() && de_run.read() &&
            ((ex_opcode.read() == OperationName::LB) || (ex_opcode.read() == OperationName::LBU) ||
             (ex_opcode.read() == OperationName::LW))) {  // execute is running load instr
            if (de_insn_use_rs.read() && (de_rs_addr.read() == ex_rd_addr.read())) {
                load_use_hazard.write(true);
            } else if (de_insn_use_rt.read() && (de_rt_addr.read() == ex_rd_addr.read())) {
                load_use_hazard.write(true);
            } else {
                load_use_hazard.write(false);
            }
        }
    }
}

void Classical_decode::measurement_valid_detection() {
    bool                       flag;
    sc_uint<G_NUM_QUBITS_LOG2> selected;
    bool*                      v_qmr_valid_all;
    bool                       v_qmr_all_valid;
    v_qmr_valid_all = new bool[m_num_qubits];

    Qasm_instruction cur_insn;

    while (true) {
        wait();

        cur_insn = de_insn.read();

        for (size_t i = 0; i < m_num_qubits; ++i) {
            v_qmr_valid_all[i] = de_qmr_valid_all[i].read();
        }

        v_qmr_all_valid = true;
        for (size_t i = 0; i < m_num_qubits; ++i) {
            v_qmr_all_valid = v_qmr_all_valid & v_qmr_valid_all[i];
        }
        de_qmr_all_valid.write(v_qmr_all_valid);

        selected = cur_insn.get_qubit_sel();
        flag     = 1;
        for (size_t i = 0; i < m_num_qubits; i++) {
            if (i == selected) flag = v_qmr_valid_all[i];
        }
        de_qmr_sel_valid.write(flag);
    }
}

void Classical_decode::measurement_result_selection() {
    bool                       flag;
    sc_uint<G_NUM_QUBITS_LOG2> selected;
    sc_uint<1>*                v_qmr_data_all;
    v_qmr_data_all = new sc_uint<1>[m_num_qubits];

    Qasm_instruction cur_insn;

    while (true) {
        wait();

        cur_insn = de_insn.read();

        for (size_t i = 0; i < m_num_qubits; ++i) {
            v_qmr_data_all[i] = de_qmr_data_all[i].read();
        }
        selected = cur_insn.get_qubit_sel();
        flag     = 1;

        for (size_t i = 0; i < m_num_qubits; i++) {
            if (i == selected) flag = v_qmr_data_all[i];
        }
        de_qmr_data.write(flag);
    }
}

void Classical_decode::fmr_ready_logic() {
    while (true) {
        wait();

        // If stalls on a FMR instruction
        if (!de_clk_en.read() && de_cl_valid.read() && de_is_fmr.read())
            de_fmr_ready_next.write(de_qmr_sel_valid.read() & !ex_meas_ena.read());
        else
            de_fmr_ready_next.write(de_qmr_all_valid.read() & !ex_meas_ena.read());
    }
}

void Classical_decode::write_output() {
    while (true) {
        wait();

        bool done = de_done.read();
        Clp2App_done.write(de_done.read());
        Clp2Ic_branching.write(de_br_start.read());
        Clp2Ic_ready.write(de_clk_en.read());
        Clp2Ic_target.write(if_target_pc.read());
        // Clp2Ic_pc.write(de_pc.read());
        Clp2MRF_meas_issue.write(ex_meas_ena.read());
    }
}

void Classical_decode::decode_stage() {
    Qasm_instruction      insn;
    sc_uint<OPCODE_WIDTH> opcode;
    sc_int<20>            imm_v;
    sc_int<21>            br_addr_v;

    while (true) {
        wait();

        if (de_insn_valid) {
            insn   = de_insn.read();
            opcode = insn.get_opcode();
            de_opcode.write(opcode);

            // if this instruction is ADD, SUB, AND, OR, XOR, NOT, LDUI, LDI, ADDC, SUBC, CMP, TEST,
            // FBR, FMR
            if (opcode >= 20 || opcode == 13) {
                de_insn_use_rd.write(true);
            } else {
                de_insn_use_rd.write(false);
            }
        }

        if (opcode == 21)
            de_is_fmr.write(true);
        else
            de_is_fmr.write(false);

        de_cl_valid.write(insn.is_cl_insn() & de_insn_valid.read() & de_run.read());
        de_br_valid.write(insn.is_cl_insn() & de_insn_valid.read() & de_run.read());
        de_q_valid.write(insn.is_q_insn() & de_insn_valid.read() & de_run.read());

        de_meas_ena.write(insn.is_meas());

        de_rs_addr.write(insn.get_rs_addr());
        de_insn_use_rs.write(insn.is_rs_used());
        de_rt_addr.write(insn.get_rt_addr());
        de_insn_use_rt.write(insn.is_rt_used());
        de_rd_addr.write(insn.get_rd_addr());
        de_uimm.write(insn.get_uimm());
        de_imm.write(insn.get_imm());
        de_br_addr.write(insn.get_br_addr());
        de_br_cond.write(insn.get_br_cond());
        // de_imm_sign.write(insn[19]);
        // de_br_addr_sign.write(insn[24]);
    }
}

void Classical_decode::write_insn_file() {
    Qasm_instruction cur_insn;

    while (true) {
        wait();

        if (de_run.read() && de_clk_en.read()) {
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

void Classical_decode::clock_counter() {
    while (true) {
        wait();
        m_num_cycles++;
    }
}

void Classical_decode::add_telf_header() {

    // Specify the signal type
    telf_os << "#Insn" << std::endl;

    telf_os << "       sim time,    Clock cycle,      addr,                         insn"
            << std::endl;
}

}  // namespace cactus
