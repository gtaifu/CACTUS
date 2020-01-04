#ifndef _EXE_FLAG_GEN_H_
#define _EXE_FLAG_GEN_H_

#include <systemc>

#include "global_json.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

#define NUM_EXE_FLAGS 4
#define NUM_HIS_MSMT_RES 3

// fast conditional execution logic
class Fce_logic : public sc_core::sc_module {
  public:  // general IO
    sc_in<bool> clock;
    sc_in<bool> reset;

    // input: measurement result for each qubit
    sc_in<sc_uint<1>> QM2MRF_qubit_data;
    sc_in<sc_uint<1>> QM2MRF_qubit_ena;

    // output
    sc_out<sc_uint<NUM_EXE_FLAGS>> out_exe_flag;

  protected:  // internal signals
    // store NUM_HIS_MSMT_RES historical msmt results
    // index 0 stores the latest result
    sc_vector<sc_signal<sc_uint<1>>> msmt_res_his;

  protected:              // internal process
    void log_msmt_his();  // push the latest msmt result into the buffer
    void gen_exe_flag();  // generate execution flag according to the msmt result trace

  public:
    Fce_logic(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Fce_logic);
};

class Exe_flag_gen : public sc_core::sc_module {
  public:  // general IO
    sc_in<bool> clock;
    sc_in<bool> reset;

    // input: measurement result for each qubit
    sc_vector<sc_in<sc_uint<1>>> QM2MRF_qubit_data;
    sc_vector<sc_in<sc_uint<1>>> QM2MRF_qubit_ena;

    // output
    sc_vector<sc_out<sc_uint<NUM_EXE_FLAGS>>> exe_flag_per_qubit;

  protected:  // internal modules
    sc_vector<Fce_logic> vec_fce_logic;

  protected:  // configuration
    unsigned int num_qubits = 0;

    void config();

  public:
    Exe_flag_gen(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Exe_flag_gen);
};

}  // namespace cactus

#endif  // _EXE_FLAG_GEN_H_
