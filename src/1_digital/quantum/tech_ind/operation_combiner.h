#ifndef _OPERATION_COMBINER_H_
#define _OPERATION_COMBINER_H_

#include <string>
#include <systemc>
#include <vector>

#include "generic_if.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "q_data_type.h"
#include "telf_module.h"
#include "util_modules.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Operation_combiner : public Telf_module {
  public:
    sc_in<bool> in_clock;
    sc_in<bool> reset;

    // vliw pipelanes output
    sc_vector<sc_in<Q_pipe_interface>> vec_in_q_pipe_interface;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:  // internal signals
    sc_signal<Q_pipe_interface> q_pipe_interface_sig;
    sc_signal<bool>             i_timestamp_match;
    sc_signal<unsigned int>     i_timestamp;

  public:
    unsigned int m_num_qubits;
    unsigned int m_vliw_width;

  public:  // methods
    void do_output();

    void detect_timestamp_match();  // used to detect operations with the same timing label

    void log_telf();  // log method

  public:
    void config();

    void add_telf_header();
    void add_telf_line();

    Operation_combiner(const sc_core::sc_module_name& n);

    ~Operation_combiner();

    SC_HAS_PROCESS(Operation_combiner);
};

}  // end of namespace cactus

#endif  // _OPERATION_COMBINER_H_
