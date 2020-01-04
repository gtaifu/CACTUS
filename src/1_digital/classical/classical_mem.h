#ifndef _CLASSICAL_MEM_H_
#define _CLASSICAL_MEM_H_

#include <systemc>

#include "data_memory.h"
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

class Classical_mem : public Telf_module {
  public:
    sc_in<bool> clock;

    // execute stage -> mem stage
    sc_in<Qasm_instruction> ex_insn;
    sc_in<bool>             ex_wr_rd_en;
    sc_in<bool>             ex_run;
    // read/write memory or not
    sc_in<bool>       ex_mem_strobe;
    sc_in<sc_int<32>> ex_rd_value;
    // read or write memory
    sc_in<bool> ex_mem_rw;
    // load data from memory
    sc_in<sc_int<32>>  ex_mem_data;
    sc_in<sc_uint<32>> ex_rd_addr;
    // access memory with byte,hal word,word type
    sc_in<MEM_ACCESS_TYPE> ex_mem_addr_sel;
    // sign extend
    sc_in<bool> ex_mem_sext;
    sc_in<bool> ex_ex2reg;

    // cmp results
    sc_vector<sc_in<bool>> flags_cmp;

    // mem stage to wb stage
    sc_out<Qasm_instruction> mem_insn;
    sc_out<bool>             mem_run;
    sc_out<sc_uint<32>>      mem_rd_addr;
    sc_out<sc_int<32>>       mem_ex_rd_value;
    sc_out<bool>             mem_wr_rd_en;
    sc_out<sc_int<32>>       mem_rd_value;
    sc_out<bool> mem_ex2reg;  // select mem data or execute data to write back rd register

    // mem stage to decode stage
    sc_vector<sc_out<bool>> flags_cmp_dly;

  public:  // internal signals
    sc_signal<sc_uint<32>>     mem_out_data;
    sc_signal<MEM_ACCESS_TYPE> mem_addr_sel;
    sc_signal<bool>            mem_sext;

  protected:  // methods
    void ex2mem_ff();
    void write_mem();
    void read_mem();
    void sign_extend();
    void clock_counter();

  public:  // member function
    void config();
    void add_telf_header();

  public:  // member variables
    std::string  m_output_dir;
    unsigned int m_data_mem_size = 0;
    int          m_num_cycles    = 0;
    Data_memory* m_data_mem;

  public:
    Classical_mem(const sc_core::sc_module_name& n);

    ~Classical_mem();

    SC_HAS_PROCESS(Classical_mem);
};

}  // namespace cactus

#endif  //_CLASSICAL_MEM_H_
