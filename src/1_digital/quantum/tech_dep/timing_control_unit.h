#ifndef _TIMING_CONTROL_H_
#define _TIMING_CONTROL_H_

#include <string>
#include <systemc>
#include <vector>

#include "event_queue_manager.h"
#include "generic_if.h"
#include "global_counter.h"
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

class Timing_control_unit : public Telf_module {
  public:
    // input
    sc_in<bool>             in_clock;
    sc_in<bool>             reset;
    sc_in<bool>             in_50MHz_clock;
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // run timing control unit
    sc_in<bool> run;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

    // whether quantum is ready to execute instruction
    sc_out<bool> out_Qp2clp_ready;

    // whether timing queue is empty
    sc_out<bool> out_eq_empty;

    // whether timing queue is almost full
    sc_out<bool> out_eq_almostfull;

  public:  // internal modules
    Event_queue_manager event_queue_manager;

  public:  // methods
    void do_output();

  public:
    void config();

    Timing_control_unit(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Timing_control_unit);
};

}  // end of namespace cactus

#endif  // _TIMING_CONTROL_H_