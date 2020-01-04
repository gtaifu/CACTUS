#ifndef _MEAS_ISSUE_GEN_H_
#define _MEAS_ISSUE_GEN_H_

#include <iterator>
#include <string>
#include <systemc>
#include <vector>

#include "generic_if.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "q_data_type.h"
#include "telf_module.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_inout;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Meas_issue_gen : public Telf_module {
  public:  // I/O port
    // in singal
    sc_in<bool> in_clock;
    sc_in<bool> reset;

    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // out measurement info
    sc_out<Generic_meas_if> out_Qp2MRF_meas_issue;

  public:
    unsigned int m_num_qubits;
    std::string  m_instruction_type;

  public:
    void config();

    // output measure issue
    void do_output();

    // output log
    void log_telf();
    void add_telf_header();
    void add_telf_line();

    Meas_issue_gen(const sc_core::sc_module_name& n);

    ~Meas_issue_gen();

    SC_HAS_PROCESS(Meas_issue_gen);
};

}  // end of namespace cactus

#endif  // _MEAS_ISSUE_GEN_H_