#ifndef _ICACHE_RTL_H_
#define _ICACHE_RTL_H_

#include <systemc.h>

#include "global_json.h"
#include "num_util.h"
#include "qasm_instruction.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Icache_rtl) {
    // input
    sc_in<bool> clock;
    sc_in<bool> reset;

    sc_in<bool>                          Clp2Ic_ready;
    sc_in<bool>                          Clp2Ic_branching;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Ic_target;

    // output
    sc_out<bool>             IC2Clp_valid;
    sc_out<bool>             IC2Clp_br_done;
    sc_out<Qasm_instruction> IC2Clp_insn;

  public:  // other signals
    int num_cycles = 0;
    // std::ofstream
    // insn_result_veri_out;

  public:  // methods
    void         init_mem_bin(std::string qisa_bin_fn);
    unsigned int convert_line_to_ele_instr(std::vector<std::vector<std::string>> & vec_instr,
                                           std::string & line_str, const std::string& line_num_str);
    void         init_mem_asm(std::string qisa_asm_fn);

    void combinational_gen();
    void register_left_logic();
    void register_right_logic();
    void register_right();
    void register_at_memory_pipeline();
    void no_register_memory_pipeline();

    void read_insn();
    void memory_out_reg();
    void no_memory_out_reg();

    void drive_cache_output();

    void clock_counter();
    // void write_output_file();

  protected:  // internal signals
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> pc;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> pca;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> pcb;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> pcc;
    sc_signal<bool>                          branch;
    sc_signal<bool>                          brancha;
    sc_signal<bool>                          branchb;
    sc_signal<bool>                          branchc;
    sc_signal<bool>                          readya;

    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> pc_reg_a;
    sc_signal<bool>                          branch_reg_a;
    sc_signal<bool>                          branch_reg_b;
    sc_signal<Qasm_instruction>              insn_reg_b;
    sc_signal<Qasm_instruction>              insn_reg_c;

    std::vector<unsigned int>             cache_mem_bin;
    std::vector<std::vector<std::string>> cache_mem_asm;
    unsigned int                          program_length;
    std::map<std::string, unsigned int>   map_label;

  public:  // member variables
    Instruction_type m_instruction_type;

  public:
    Icache_rtl(const sc_core::sc_module_name& n)
        : sc_core::sc_module(n) {

        Global_config& global_config = Global_config::get_instance();
        m_instruction_type           = global_config.instruction_type;

        auto logger = get_logger_or_exit("cache_logger");
        logger->trace("Start initializing {}...", this->name());

        if (global_config.instruction_type == Instruction_type::BIN) {
            init_mem_bin(global_config.qisa_bin_fn);
        } else {
            init_mem_asm(global_config.qisa_asm_fn);
        }

        SC_THREAD(combinational_gen);
        sensitive << Clp2Ic_target << Clp2Ic_branching << Clp2Ic_ready << pcc << branchc << readya
                  << pca << brancha;

        SC_THREAD(register_right);
        sensitive << pc << branch << Clp2Ic_ready;

        SC_CTHREAD(register_left_logic, clock.pos());
        SC_CTHREAD(register_right_logic, clock.pos());
        SC_CTHREAD(register_at_memory_pipeline, clock.pos());

        SC_THREAD(no_register_memory_pipeline);
        sensitive << pc << branch;

        SC_CTHREAD(read_insn, clock.pos());
        SC_CTHREAD(memory_out_reg, clock.pos());
        SC_THREAD(no_memory_out_reg);
        sensitive << branch_reg_a;

        SC_THREAD(drive_cache_output);
        sensitive << insn_reg_c << branch_reg_b;

        SC_CTHREAD(clock_counter, clock.pos());
        // SC_CTHREAD(write_output_file, clock.pos());

        logger->trace("Finished initializing {}...", this->name());
    }

    SC_HAS_PROCESS(Icache_rtl);
};
}  // namespace cactus

#endif  // _ICACHE_RTL_H_
