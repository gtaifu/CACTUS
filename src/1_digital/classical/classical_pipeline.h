#ifndef _CLASSICAL_PIPELINE_H_
#define _CLASSICAL_PIPELINE_H_

#include <systemc>

#include "classical_decode.h"
#include "classical_execute.h"
#include "classical_fetch.h"
#include "classical_mem.h"
#include "classical_wb.h"
#include "qasm_instruction.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_int;
using sc_dt::sc_uint;

class Classical_pipeline : public Telf_module {
  public:
    sc_in<bool> clock;
    sc_in<bool> reset;
    // this signal indicates the signal_initiate process, which initiate relevant signals
    sc_in<bool> init;

    // interface to the user
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;
    sc_out<bool>                         Clp2App_done;

    // instructions sent from classical pipeline to quantum pipeline, assisted by handshaking.
    sc_in<bool>                 Qp2Clp_ready;
    sc_out<bool>                Clp2Qp_valid;
    sc_out<Qasm_instruction>    Clp2Qp_insn;
    sc_out<sc_uint<INSN_WIDTH>> Clp2Qp_Rs;

    // instructions sent from icache to classical pipeline, assisted by handshaking.
    sc_out<bool>            Clp2Ic_ready;
    sc_in<bool>             IC2Clp_valid;
    sc_in<bool>             IC2Clp_branch;
    sc_in<Qasm_instruction> insn;

    // branch related signals to the instruction cache
    sc_out<bool>                          Clp2Ic_branching;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Ic_target;

    // feedback control related interface
    sc_out<bool>                 Clp2MRF_meas_issue;
    sc_in<bool>                  MRF2Clp_ready;
    sc_vector<sc_in<sc_uint<1>>> MRF2Clp_data;
    sc_vector<sc_in<sc_uint<1>>> MRF2Clp_valid;

  public:  // internal signals
    // instruction fetch stage
    sc_signal<Qasm_instruction>              if_insn;
    sc_signal<bool>                          if_reset;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> if_init_pc;
    sc_signal<bool>                          if_done;
    sc_signal<bool>                          if_reset_dly;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> if_normal_pc;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> if_target_pc;
    sc_signal<bool>                          if_run;
    sc_signal<bool>                          if_br_done;
    sc_signal<bool>                          if_br_done_valid;

    // decode stage
    sc_signal<bool>                          de_reset;
    sc_signal<bool>                          de_done;
    sc_signal<Qasm_instruction>              de_insn;
    sc_signal<bool>                          de_insn_valid;
    sc_signal<bool>                          de_run;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> de_pc;
    sc_signal<bool>                          de_reset_dly;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> de_init_pc;
    sc_signal<bool>                          de_br_start;

    sc_signal<sc_uint<OPCODE_WIDTH>> de_opcode;
    sc_signal<bool>                  de_qp_ready;
    sc_signal<bool>                  de_br_valid;
    sc_signal<bool>                  de_q_valid;
    sc_signal<bool>                  de_qisa_valid;

    sc_signal<sc_uint<32>> de_rs_addr;
    sc_signal<sc_uint<32>> de_rt_addr;
    sc_signal<sc_uint<32>> de_rd_addr;
    sc_signal<sc_uint<15>> de_uimm;
    sc_signal<sc_int<20>>  de_imm;
    sc_signal<sc_int<21>>  de_br_addr;
    sc_signal<sc_uint<32>> de_br_cond;

    sc_signal<bool> de_insn_use_rd;
    sc_signal<bool> de_clk_en;  // signal indicates that the pipeline is stalled or not
    sc_signal<bool> de_stall;

    sc_signal<bool> de_qmr_data;
    sc_signal<bool> de_fmr_ready;
    sc_signal<bool> de_fmr_ready_next;
    sc_signal<bool> de_meas_ena;

    // execute stage
    sc_signal<Qasm_instruction>      ex_insn;
    sc_signal<bool>                  ex_run;
    sc_signal<sc_uint<OPCODE_WIDTH>> ex_opcode;
    sc_signal<bool>                  ex_wr_rd_en;
    sc_signal<sc_int<32>>            ex_rd_value;
    sc_signal<bool>                  ex_mem_strobe;
    sc_signal<bool>                  ex_mem_rw;
    sc_signal<sc_int<32>>            ex_mem_data;
    sc_signal<sc_uint<32>>           ex_rd_addr;
    sc_signal<MEM_ACCESS_TYPE>       ex_mem_addr_sel;
    sc_signal<bool>                  ex_mem_sext;
    sc_signal<bool>                  ex_ex2reg;
    sc_signal<bool>                  ex_meas_ena;

    sc_vector<sc_signal<bool>> flags_cmp_dly;
    sc_vector<sc_signal<bool>> flags_cmp;

    // mem stage
    sc_signal<Qasm_instruction> mem_insn;
    sc_signal<bool>             mem_run;
    sc_signal<sc_uint<32>>      mem_rd_addr;
    sc_signal<sc_int<32>>       mem_ex_rd_value;
    sc_signal<bool>             mem_wr_rd_en;
    sc_signal<sc_int<32>>       mem_rd_value;
    sc_signal<bool>             mem_ex2reg;

    // wb stage
    sc_signal<bool>                              wb_run;
    sc_signal<sc_uint<32>>                       wb_rd_addr;
    sc_signal<bool>                              wb_wr_rd_en;
    sc_signal<sc_int<32>>                        wb_rd_value;
    sc_vector<sc_signal<sc_int<REG_FILE_WIDTH>>> reg_file;

  public:  // sub modules
    Classical_fetch   clp_fetch;
    Classical_decode  clp_decode;
    Classical_execute clp_execute;
    Classical_mem     clp_mem;
    Classical_wb      clp_wb;

  public:  // member variables
    unsigned int m_num_qubits;

  public:
    void config();

    Classical_pipeline(const sc_core::sc_module_name& n);

    ~Classical_pipeline();

    SC_HAS_PROCESS(Classical_pipeline);
};
}  // namespace cactus

#endif
