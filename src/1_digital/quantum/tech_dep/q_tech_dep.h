#ifndef _Q_TECH_DEP_H_
#define _Q_TECH_DEP_H_

#include <iterator>
#include <string>
#include <systemc>
#include <vector>

#include "fast_conditional_execution.h"
#include "generic_if.h"
#include "global_json.h"
#include "q_data_type.h"
#include "timing_control_unit.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_inout;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Q_tech_dep : public Telf_module {
  public:
    sc_in<bool> in_clock;
    sc_in<bool> reset;

    // clock used in timing control unit
    sc_in<bool> in_50MHz_clock;

    // connect to event queue manager
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // trigger timing control unit run signal
    sc_in<bool> run;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

    // whether quantum is ready to execute instruction
    sc_out<bool> out_Qp2clp_ready;

    // whether timing queue is empty
    sc_out<bool> out_eq_empty;

    // whether timing queue is almost full
    sc_out<bool> out_eq_almostfull;

    // whether measurement operation is cancel
    sc_out<Generic_meas_if> out_Qp2MRF_meas_cancel;
    // sc_signal<Generic_meas_if> out_Qp2MRF_meas_cancel;

  public:
    // internal modules
    Timing_control_unit        timing_control_unit;
    Fast_conditional_execution fast_cond_exe;

    // internal signals
    sc_signal<Q_pipe_interface> q_pipe_interface_sig;

  public:
    void config();

    Q_tech_dep(const sc_core::sc_module_name& n);
};

}  // end of namespace cactus

#endif  // _Q_TECH_DEP_H_