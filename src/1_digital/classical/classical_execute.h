#ifndef _CLASSICAL_EXECUTE_H_
#define _CLASSICAL_EXECUTE_H_

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

class Classical_execute : public Telf_module {
  public:
    sc_in<bool> clock;
    // this signal indicates the signal_initiate process, which initiate relevant signals
    sc_in<bool> init;

    // instructions sent from classical pipeline to quantum pipeline, assisted by handshaking.
    sc_out<bool>                Clp2Qp_valid;
    sc_out<Qasm_instruction>    Clp2Qp_insn;
    sc_out<sc_uint<INSN_WIDTH>> Clp2Qp_Rs;

    // decode stage -> execute stage
    sc_in<Qasm_instruction>              de_insn;
    sc_in<bool>                          de_stall;
    sc_in<bool>                          de_q_valid;
    sc_in<bool>                          de_meas_ena;
    sc_in<bool>                          de_run;
    sc_in<bool>                          de_fmr_ready_next;
    sc_in<bool>                          de_qmr_data;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> de_pc;
    sc_in<sc_uint<OPCODE_WIDTH>>         de_opcode;
    sc_in<sc_uint<32>>                   de_rs_addr;
    sc_in<sc_uint<32>>                   de_rt_addr;
    sc_in<sc_uint<32>>                   de_rd_addr;
    sc_in<sc_uint<15>>                   de_uimm;
    sc_in<sc_int<20>>                    de_imm;
    sc_in<sc_int<21>>                    de_br_addr;
    sc_in<sc_uint<32>>                   de_br_cond;
    sc_in<bool>                          de_insn_use_rd;

    // memory stage to execute stage
    // register value forwarding
    sc_in<bool>        mem_run;
    sc_in<sc_uint<32>> mem_rd_addr;
    sc_in<bool>        mem_wr_rd_en;
    sc_in<sc_int<32>>  mem_ex_rd_value;

    // wb stage to execute stage
    // register file
    sc_vector<sc_in<sc_int<REG_FILE_WIDTH>>> reg_file;

    // register value forwarding
    sc_in<bool>        wb_run;
    sc_in<sc_uint<32>> wb_rd_addr;
    sc_in<bool>        wb_wr_rd_en;
    sc_in<sc_int<32>>  wb_rd_value;

    // execute stage -> decode stage
    sc_out<bool>                  ex_meas_ena;
    sc_out<bool>                  de_fmr_ready;
    sc_out<bool>                  de_done;
    sc_out<sc_uint<OPCODE_WIDTH>> ex_opcode;

    // execute stage to mem stage
    sc_vector<sc_out<bool>> flags_cmp;
    sc_out<bool>            ex_wr_rd_en;
    sc_out<bool>            ex_run;
    // read/write memory or not
    sc_out<bool>        ex_mem_strobe;
    sc_out<sc_int<32>>  ex_rd_value;
    sc_out<sc_uint<32>> ex_rd_addr;
    // read or write memory
    sc_out<bool> ex_mem_rw;
    // load data from memory
    sc_out<sc_int<32>> ex_mem_data;
    // access memory with byte,hal word,word type
    sc_out<MEM_ACCESS_TYPE> ex_mem_addr_sel;
    // sign extend
    sc_out<bool>             ex_mem_sext;
    sc_out<bool>             ex_ex2reg;
    sc_out<Qasm_instruction> ex_insn;

  public:  // internal signals
    sc_signal<bool>                          ex_q_valid;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> ex_pc;
    sc_signal<bool>                          ex_qmr_data;
    sc_signal<sc_uint<32>>                   ex_rs_addr;
    sc_signal<sc_uint<32>>                   ex_rt_addr;
    sc_signal<sc_uint<15>>                   ex_uimm;
    sc_signal<sc_int<20>>                    ex_imm;
    sc_signal<sc_int<21>>                    ex_br_addr;
    sc_signal<sc_uint<32>>                   ex_br_cond;
    sc_signal<bool>                          ex_insn_use_rd;

    sc_vector<sc_signal<bool>> flags_test;
    sc_signal<bool>            ex_cond_result;

  protected:  // methods
    void de2ex_ff();
    void write_to_qp();
    void execution();
    void clock_counter();
    void write_insn_file();

  public:  // member function
    void config();
    void add_telf_header();

  public:  // member variables
    unsigned int m_num_qubits = 0;
    int          m_num_cycles = 0;

  public:
    Classical_execute(const sc_core::sc_module_name& n);

    ~Classical_execute();

    SC_HAS_PROCESS(Classical_execute);
};

}  // namespace cactus

#endif  //_CLASSICAL_EXECUTE_H_
