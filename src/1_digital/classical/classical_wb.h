#ifndef _CLASSICAL_WB_H_
#define _CLASSICAL_WB_H_

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

class Classical_wb : public Telf_module {
  public:
    sc_in<bool> clock;
    // this signal indicates the signal_initiate process, which initiate relevant signals
    sc_in<bool> init;

    // mem stage to wb stage
    sc_in<Qasm_instruction> mem_insn;
    sc_in<bool>             mem_run;
    sc_in<sc_uint<32>>      mem_rd_addr;
    sc_in<sc_int<32>>       mem_ex_rd_value;
    sc_in<bool>             mem_wr_rd_en;
    sc_in<sc_int<32>>       mem_rd_value;
    sc_in<bool> mem_ex2reg;  // select mem data or execute data to write back rd register

    // wb stage to execute stage, mainly for update register file and
    sc_out<bool>                              wb_run;
    sc_out<sc_uint<32>>                       wb_rd_addr;
    sc_out<bool>                              wb_wr_rd_en;
    sc_out<sc_int<32>>                        wb_rd_value;
    sc_vector<sc_out<sc_int<REG_FILE_WIDTH>>> reg_file;

  public:  // internal signals
    sc_signal<Qasm_instruction> wb_insn;

  protected:  // methods
    void mem2wb_ff();
    void write_reg_file();
    void clock_counter();

  public:  // member function
    void config();
    void add_telf_header();

  public:  // member variables
    std::string m_output_dir;
    int         m_num_cycles = 0;

  public:
    Classical_wb(const sc_core::sc_module_name& n);

    ~Classical_wb();

    SC_HAS_PROCESS(Classical_wb);
};

}  // namespace cactus

#endif  //_CLASSICAL_WB_H_
