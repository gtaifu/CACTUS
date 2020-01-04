#ifndef _CLASSICAL_PART_H_
#define _CLASSICAL_PART_H_

#include <string>
#include <systemc>

#include "classical_pipeline.h"
#include "global_json.h"
#include "icache_top.h"
#include "logger_wrapper.h"
#include "meas_reg_file_top.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Classical_part) {
  public:  // IO ports
    sc_in<bool> in_clock;
    sc_in<bool> reset;

    // this signal indicates the signal_initiate process, which initiate relevant signals
    sc_in<bool> init;

    // interface to the user
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;
    sc_out<bool>                         Clp2App_done;

    // instructions sent from classical pipeline to quantum pipeline
    // assisted by handshaking.
    sc_in<bool>                 Qp2Clp_ready;
    sc_out<bool>                Clp2Qp_valid;
    sc_out<Qasm_instruction>    Clp2Qp_insn;
    sc_out<sc_uint<INSN_WIDTH>> Clp2Qp_Rs;

    // interface to quantum pipeline
    sc_in<Generic_meas_if> Qp2MRF_meas_issue;
    sc_in<Generic_meas_if> Qp2MRF_meas_cancel;

    // quantum measurement device interface
    sc_in<Generic_meas_if> Qm2MRF_meas_result;

  public:
    void init_mem_asm(std::string asm_fn);

  protected:  // sub-modules
    Classical_pipeline classical_pipeline;
    Meas_reg_file      meas_reg_file;
    ICache             icache;

  protected:  // internal signals
    // feedback control related signals
    sc_signal<bool>                  Clp2MRF_meas_issue;
    sc_signal<bool>                  MRF2Clp_ready;
    sc_vector<sc_signal<sc_uint<1>>> MRF2Clp_valid;
    sc_vector<sc_signal<sc_uint<1>>> MRF2Clp_data;

    // this group contains signals to the cache
    sc_signal<bool>             Clp2Ic_ready;
    sc_signal<bool>             IC2Clp_valid;
    sc_signal<Qasm_instruction> IC2Clp_insn;
    sc_signal<bool>             IC2Clp_branch_done;

    sc_signal<bool>                          Clp2Ic_branching;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Ic_target;

  protected:
    unsigned int num_qubits = 0;

    void config();

  public:
    Classical_part(const sc_core::sc_module_name& n);

    ~Classical_part();
};
}  // end of namespace cactus

#endif  // _CLASSICAL_PART_H_
