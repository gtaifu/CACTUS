#ifndef _EVENT_QUEUE_MANAGER_H_
#define _EVENT_QUEUE_MANAGER_H_

#include <string>
#include <systemc>
#include <vector>

#include "generic_if.h"
#include "global_counter.h"
#include "global_json.h"
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

class Event_queue_manager : public Telf_module {
  public:
    // input
    sc_in<bool> in_clock;
    sc_in<bool> reset;
    sc_in<bool> in_50MHz_clock;

    // trigger timing control unit run signal
    sc_in<bool> run;

    // input event
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // event queue whether is in error state
    sc_out<bool> out_error_state;

    // event queue whether is almost full
    sc_out<bool> out_eq_almostfull;

    // output event
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:
    // event fifo
    sc_fifo<Q_pipe_interface> event_queue;

    // internal signals
    sc_signal<bool> counter_finished_sig;  // ready to output event
    sc_signal<bool> event_queue_read_sig;  // read event fifo signal
    sc_signal<bool> counter_start;         // start to count of each event
    sc_signal<bool> counter_running;       // counter is running

    sc_signal<unsigned int>     counter;                    // current counter value
    sc_signal<unsigned int>     target_count_value;         // target counter value
    sc_signal<Q_pipe_interface> q_pipe_interface_sig;       // event sig
    sc_signal<Q_pipe_interface> q_pipe_interface_next_sig;  // next output event

    sc_signal<bool> i_run;          // clock synchronized run signal
    sc_signal<bool> i_run_old;      // delayed a clock cycle of run signal
    sc_signal<bool> i_run_pos;      // indicate a rising edge of run signal
    sc_signal<bool> i_run_pos_old;  // delayed a clock cycle of run signal posedge

    sc_signal<bool> last_but_one;  // whether reached the last one count of each event

    sc_signal<bool> eq_empty;       // event queue empty
    sc_signal<bool> i_error_state;  // indicate event queue manager state

  public:
    unsigned int m_num_qubits;

  public:
    // methods
    void read_fifo();
    void write_fifo();

    // detect run signal posedge
    void generate_run_pos_sig();

    // generate event queue read request
    void generate_event_queue_read_sig();

    // detect rising edge of run signal and event queue manager error state
    void counter_control();

    // generate count start of each event
    void generate_counter_start();

    // output event queue state
    void write_state();

    // generate counter finished signal
    void generate_count_finish_sig();

    // output signal
    void do_output();

    // write log
    void log_telf();

  public:
    void config();

    void add_telf_header();
    void add_telf_line();

  public:
    Event_queue_manager(const sc_core::sc_module_name& n);

    ~Event_queue_manager();

    SC_HAS_PROCESS(Event_queue_manager);
};

}  // end of namespace cactus

#endif  // _EVENT_QUEUE_MANAGER_H_