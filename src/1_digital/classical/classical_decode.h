#ifndef _CLASSICAL_DECODE_H_
#define _CLASSICAL_DECODE_H_

#include <systemc>

#include "global_counter.h"
#include "global_json.h"
#include "q_data_type.h"
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

class Classical_decode : public Telf_module {
  public:
    sc_in<bool> clock;
    // this signal indicates the signal_initiate process, which initiate relevant signals
    sc_in<bool> init;

    // interface to the user
    sc_out<bool> Clp2App_done;

    // branch related signals to the instruction cache
    sc_out<bool>                          Clp2Ic_ready;
    sc_out<bool>                          Clp2Ic_branching;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Ic_target;

    // feedback control related interface
    sc_out<bool>                 Clp2MRF_meas_issue;
    sc_in<bool>                  MRF2Clp_ready;
    sc_vector<sc_in<sc_uint<1>>> MRF2Clp_data;
    sc_vector<sc_in<sc_uint<1>>> MRF2Clp_valid;

    // fetch stage -> decode stage
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> if_init_pc;
    sc_in<bool>                          if_reset;
    sc_in<bool>                          if_reset_dly;
    sc_in<bool>                          if_run;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> if_normal_pc;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> if_target_pc;
    sc_in<bool>                          de_qp_ready;
    sc_in<bool>                          if_br_done;
    sc_in<bool>                          if_br_done_valid;
    sc_in<Qasm_instruction>              de_insn;
    sc_in<bool>                          de_insn_valid;

    // execute stage -> decode stage
    sc_vector<sc_in<bool>>       flags_cmp_dly;
    sc_in<bool>                  ex_meas_ena;
    sc_in<bool>                  de_done;
    sc_in<bool>                  de_fmr_ready;
    sc_in<bool>                  ex_run;
    sc_in<sc_uint<OPCODE_WIDTH>> ex_opcode;
    sc_in<sc_uint<32>>           ex_rd_addr;

    // decode stage -> execute stage, fetch stage
    sc_out<bool>                          de_run;
    sc_out<bool>                          de_meas_ena;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> de_init_pc;
    sc_out<bool>                          de_reset;
    sc_out<bool>                          de_reset_dly;
    sc_out<bool>                          de_br_start;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> de_pc;
    sc_out<sc_uint<32>>                   de_rs_addr;
    sc_out<sc_uint<32>>                   de_rt_addr;
    sc_out<sc_uint<32>>                   de_rd_addr;
    sc_out<sc_uint<15>>                   de_uimm;
    sc_out<sc_int<20>>                    de_imm;
    sc_out<sc_int<21>>                    de_br_addr;
    sc_out<sc_uint<32>>                   de_br_cond;

    sc_out<bool>                  de_stall;
    sc_out<bool>                  de_clk_en;
    sc_out<bool>                  de_qmr_data;
    sc_out<bool>                  de_fmr_ready_next;
    sc_out<sc_uint<OPCODE_WIDTH>> de_opcode;
    sc_out<bool>                  de_insn_use_rd;
    sc_out<bool>                  de_q_valid;

    // internal signals
    sc_signal<bool>                  de_qmr_ready;
    sc_vector<sc_signal<sc_uint<1>>> de_qmr_data_all;
    sc_vector<sc_signal<sc_uint<1>>> de_qmr_valid_all;
    sc_signal<bool>                  de_fmr_ready_lock;
    sc_signal<bool>                  de_cl_valid;
    sc_signal<bool>                  de_br_valid;
    sc_signal<bool>                  de_is_fmr;
    sc_signal<bool>                  de_qmr_sel_valid;
    sc_signal<bool>                  de_qmr_all_valid;

    sc_signal<bool> load_use_hazard;
    sc_signal<bool> de_insn_use_rs;
    sc_signal<bool> de_insn_use_rt;

  protected:  // methods
    void mrf_in();
    void if2de_ff();
    void write_output();
    void decode_stage();
    void br_start();
    void stalling_logic();
    void measurement_valid_detection();
    void measurement_result_selection();
    void fmr_ready_logic();
    void fmr_interlocking_logic();
    void hazard_detection();

    void clock_counter();
    void write_insn_file();

  public:  // member variables
    unsigned int m_num_qubits = 0;
    int          m_num_cycles = 0;

  public:  // member function
    void config();
    void add_telf_header();

  public:
    Classical_decode(const sc_core::sc_module_name& n);

    ~Classical_decode();

    SC_HAS_PROCESS(Classical_decode);
};

}  // namespace cactus

#endif  //_CLASSICAL_DECODE_H_
