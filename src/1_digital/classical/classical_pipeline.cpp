#include "classical_pipeline.h"

#include <sstream>

#include "num_util.h"

namespace cactus {

void Classical_pipeline::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
};

Classical_pipeline::Classical_pipeline(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , clp_fetch("classical_fetch")
    , clp_decode("classical_decode")
    , clp_execute("classical_execute")
    , clp_mem("classical_mem")
    , clp_wb("classical_wb") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    MRF2Clp_data.init(m_num_qubits);
    MRF2Clp_valid.init(m_num_qubits);
    reg_file.init(REG_FILE_NUM);
    flags_cmp_dly.init(16);
    flags_cmp.init(16);

    // ----------------------------------------------------------------------------------------
    // fetch stage
    // ----------------------------------------------------------------------------------------
    clp_fetch.clock(clock);
    clp_fetch.reset(reset);

    //-------------------------input-----------------------------------------------------------
    // user -> fetch stage
    clp_fetch.App2Clp_init_pc(App2Clp_init_pc);
    clp_fetch.Qp2Clp_ready(Qp2Clp_ready);

    // icache -> fetch stage
    clp_fetch.IC2Clp_valid(IC2Clp_valid);
    clp_fetch.IC2Clp_branch(IC2Clp_branch);
    clp_fetch.insn(insn);

    // decode stage -> fetch stage
    clp_fetch.de_run(de_run);
    clp_fetch.de_pc(de_pc);
    clp_fetch.de_reset(de_reset);
    clp_fetch.de_reset_dly(de_reset_dly);
    clp_fetch.de_clk_en(de_clk_en);
    clp_fetch.de_init_pc(de_init_pc);
    clp_fetch.de_br_start(de_br_start);

    //-------------------------output----------------------------------------------------------
    // fetch stage -> decode stage
    clp_fetch.if_init_pc(if_init_pc);
    clp_fetch.if_reset(if_reset);
    clp_fetch.if_reset_dly(if_reset_dly);
    clp_fetch.if_br_done(if_br_done);
    clp_fetch.if_br_done_valid(if_br_done_valid);
    clp_fetch.if_normal_pc(if_normal_pc);
    clp_fetch.if_target_pc(if_target_pc);
    clp_fetch.de_insn(de_insn);
    clp_fetch.de_insn_valid(de_insn_valid);
    clp_fetch.if_run(if_run);
    clp_fetch.de_qp_ready(de_qp_ready);

    // ----------------------------------------------------------------------------------------
    // decoder stage
    // ----------------------------------------------------------------------------------------

    clp_decode.clock(clock);
    clp_decode.init(init);

    //-------------------------input----------------------------------------------------------
    // fetch stage -> decode stage
    clp_decode.if_init_pc(if_init_pc);
    clp_decode.if_reset(if_reset);
    clp_decode.if_reset_dly(if_reset_dly);
    clp_decode.if_br_done(if_br_done);
    clp_decode.if_br_done_valid(if_br_done_valid);
    clp_decode.if_normal_pc(if_normal_pc);
    clp_decode.if_target_pc(if_target_pc);
    clp_decode.de_insn(de_insn);
    clp_decode.de_insn_valid(de_insn_valid);
    clp_decode.if_run(if_run);
    clp_decode.de_qp_ready(de_qp_ready);

    // execute stage -> decode stage
    clp_decode.ex_meas_ena(ex_meas_ena);
    clp_decode.de_done(de_done);
    clp_decode.de_fmr_ready(de_fmr_ready);
    clp_decode.ex_run(ex_run);
    clp_decode.ex_opcode(ex_opcode);
    clp_decode.ex_rd_addr(ex_rd_addr);

    // mem stage -> decode stage
    clp_decode.flags_cmp_dly(flags_cmp_dly);

    // meas_reg -> decode stage
    clp_decode.MRF2Clp_ready(MRF2Clp_ready);
    clp_decode.MRF2Clp_data(MRF2Clp_data);
    clp_decode.MRF2Clp_valid(MRF2Clp_valid);

    //-------------------------output----------------------------------------------------------
    // decode stage -> user
    clp_decode.Clp2App_done(Clp2App_done);

    // decode stage -> fetch stage
    clp_decode.de_run(de_run);
    clp_decode.de_pc(de_pc);
    clp_decode.de_reset(de_reset);
    clp_decode.de_reset_dly(de_reset_dly);
    clp_decode.de_clk_en(de_clk_en);
    clp_decode.de_init_pc(de_init_pc);
    clp_decode.de_br_start(de_br_start);

    // decode stage -> execute stage
    // clp_decode.de_run(de_run); //already connect
    // clp_decode.de_pc(de_pc); //already connect
    // clp_execute.de_insn(de_insn); //already connect
    clp_decode.de_meas_ena(de_meas_ena);
    clp_decode.de_rs_addr(de_rs_addr);
    clp_decode.de_rt_addr(de_rt_addr);
    clp_decode.de_rd_addr(de_rd_addr);
    clp_decode.de_uimm(de_uimm);
    clp_decode.de_imm(de_imm);
    clp_decode.de_br_addr(de_br_addr);
    clp_decode.de_br_cond(de_br_cond);
    clp_decode.de_stall(de_stall);
    clp_decode.de_qmr_data(de_qmr_data);
    clp_decode.de_fmr_ready_next(de_fmr_ready_next);
    clp_decode.de_opcode(de_opcode);
    clp_decode.de_insn_use_rd(de_insn_use_rd);
    clp_decode.de_q_valid(de_q_valid);

    // decode stage -> icache
    clp_decode.Clp2Ic_ready(Clp2Ic_ready);
    clp_decode.Clp2Ic_branching(Clp2Ic_branching);
    clp_decode.Clp2Ic_target(Clp2Ic_target);

    // decode stage -> meas_reg
    clp_decode.Clp2MRF_meas_issue(Clp2MRF_meas_issue);

    // ----------------------------------------------------------------------------------------
    // execute stage
    // ----------------------------------------------------------------------------------------

    clp_execute.clock(clock);
    clp_execute.init(init);

    //-------------------------input----------------------------------------------------------
    // decode stage -> execute stage
    clp_execute.de_insn(de_insn);
    clp_execute.de_run(de_run);
    clp_execute.de_pc(de_pc);
    clp_execute.de_meas_ena(de_meas_ena);
    clp_execute.de_rs_addr(de_rs_addr);
    clp_execute.de_rt_addr(de_rt_addr);
    clp_execute.de_rd_addr(de_rd_addr);
    clp_execute.de_uimm(de_uimm);
    clp_execute.de_imm(de_imm);
    clp_execute.de_br_addr(de_br_addr);
    clp_execute.de_br_cond(de_br_cond);
    clp_execute.de_stall(de_stall);
    clp_execute.de_qmr_data(de_qmr_data);
    clp_execute.de_fmr_ready_next(de_fmr_ready_next);
    clp_execute.de_opcode(de_opcode);
    clp_execute.de_insn_use_rd(de_insn_use_rd);
    clp_execute.de_q_valid(de_q_valid);

    // mem stage -> execute stage
    clp_execute.mem_run(mem_run);
    clp_execute.mem_rd_addr(mem_rd_addr);
    clp_execute.mem_wr_rd_en(mem_wr_rd_en);
    clp_execute.mem_ex_rd_value(mem_ex_rd_value);

    // wb stage -> execute stage
    clp_execute.wb_run(wb_run);
    clp_execute.wb_rd_addr(wb_rd_addr);
    clp_execute.wb_wr_rd_en(wb_wr_rd_en);
    clp_execute.wb_rd_value(wb_rd_value);
    clp_execute.reg_file(reg_file);

    //-------------------------output----------------------------------------------------------
    // execute stage -> decode stage
    clp_execute.ex_meas_ena(ex_meas_ena);
    clp_execute.de_done(de_done);
    clp_execute.de_fmr_ready(de_fmr_ready);
    clp_execute.ex_run(ex_run);
    clp_execute.ex_opcode(ex_opcode);
    clp_execute.ex_rd_addr(ex_rd_addr);

    // execute stage -> mem stage
    clp_execute.ex_insn(ex_insn);
    clp_execute.ex_wr_rd_en(ex_wr_rd_en);
    // clp_mem.ex_run(ex_run); //already connect
    clp_execute.ex_mem_strobe(ex_mem_strobe);
    clp_execute.ex_rd_value(ex_rd_value);
    clp_execute.ex_mem_rw(ex_mem_rw);
    clp_execute.ex_mem_data(ex_mem_data);
    // clp_mem.ex_rd_addr(ex_rd_addr); //already connect
    clp_execute.ex_mem_addr_sel(ex_mem_addr_sel);
    clp_execute.ex_mem_sext(ex_mem_sext);
    clp_execute.ex_ex2reg(ex_ex2reg);
    clp_execute.flags_cmp(flags_cmp);

    // execute stage -> quantum pipeline
    clp_execute.Clp2Qp_valid(Clp2Qp_valid);
    clp_execute.Clp2Qp_insn(Clp2Qp_insn);
    clp_execute.Clp2Qp_Rs(Clp2Qp_Rs);

    // ----------------------------------------------------------------------------------------
    // mem stage
    // ----------------------------------------------------------------------------------------
    clp_mem.clock(clock);

    //-------------------------input-----------------------------------------------------------
    // excute stage to mem stage
    clp_mem.ex_insn(ex_insn);
    clp_mem.ex_wr_rd_en(ex_wr_rd_en);
    clp_mem.ex_run(ex_run);
    clp_mem.ex_mem_strobe(ex_mem_strobe);
    clp_mem.ex_rd_value(ex_rd_value);
    clp_mem.ex_mem_rw(ex_mem_rw);
    clp_mem.ex_mem_data(ex_mem_data);
    clp_mem.ex_rd_addr(ex_rd_addr);
    clp_mem.ex_mem_addr_sel(ex_mem_addr_sel);
    clp_mem.ex_mem_sext(ex_mem_sext);
    clp_mem.ex_ex2reg(ex_ex2reg);
    clp_mem.flags_cmp(flags_cmp);

    //-------------------------output----------------------------------------------------------
    // mem stage -> decode stage
    clp_mem.flags_cmp_dly(flags_cmp_dly);

    // mem stage -> execute stage
    clp_mem.mem_run(mem_run);
    clp_mem.mem_rd_addr(mem_rd_addr);
    clp_mem.mem_wr_rd_en(mem_wr_rd_en);
    clp_mem.mem_ex_rd_value(mem_ex_rd_value);

    // mem stage -> wb stage
    // clp_mem.mem_run(mem_run);  // already connect
    // clp_mem.mem_rd_addr(mem_rd_addr); // already connect
    // clp_mem.mem_ex_rd_value(mem_ex_rd_value); // already connect
    // clp_mem.mem_wr_rd_en(mem_wr_rd_en); // already connect
    clp_mem.mem_rd_value(mem_rd_value);
    clp_mem.mem_ex2reg(mem_ex2reg);
    clp_mem.mem_insn(mem_insn);

    // ----------------------------------------------------------------------------------------
    // wb stage
    // ----------------------------------------------------------------------------------------
    clp_wb.clock(clock);
    clp_wb.init(init);

    //-------------------------input----------------------------------------------------------
    // mem stage -> wb stage
    clp_wb.mem_insn(mem_insn);
    clp_wb.mem_run(mem_run);
    clp_wb.mem_rd_addr(mem_rd_addr);
    clp_wb.mem_ex_rd_value(mem_ex_rd_value);
    clp_wb.mem_wr_rd_en(mem_wr_rd_en);
    clp_wb.mem_rd_value(mem_rd_value);
    clp_wb.mem_ex2reg(mem_ex2reg);

    //-------------------------output-----------------------------------------------------------
    // wb stage -> execute stage
    clp_wb.wb_run(wb_run);
    clp_wb.wb_rd_addr(wb_rd_addr);
    clp_wb.wb_wr_rd_en(wb_wr_rd_en);
    clp_wb.wb_rd_value(wb_rd_value);
    clp_wb.reg_file(reg_file);

    logger->trace("Finished initializing {}...", this->name());
}

Classical_pipeline::~Classical_pipeline() {}

}  // namespace cactus
