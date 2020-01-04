#ifndef _MSMT_RESULT_GEN_H_
#define _MSMT_RESULT_GEN_H_

#include <string>
#include <systemc>

#include "generic_if.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "q_data_type.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Msmt_result_gen : public Telf_module {
  public:  // general IO
    sc_in<bool> in_50MHz_clock;
    sc_in<bool> reset;

    // interface to CCL
    sc_out<Generic_meas_if> out_meas_result;

    // interface to quantum simulator
    sc_in<Res_from_qsim> msmt_res;

  public:
    unsigned int m_num_qubits;

  public:
    // This method converts simulator result to Generic_meas_if, each for one qubit.
    void gen_msmt_result();

    // logging method
    void log_telf();

    void config();
    void add_telf_header();
    void add_telf_line();

  public:
    Msmt_result_gen(const sc_core::sc_module_name& n);

    ~Msmt_result_gen();

    SC_HAS_PROCESS(Msmt_result_gen);
};

}  // namespace cactus

#endif  //
