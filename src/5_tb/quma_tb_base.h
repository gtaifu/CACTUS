/*

Purpose:
========
    It defines the class Quma_tb_base, which forms the base of testbenches that are used
  to test CACTUS at different levels in the module hierarchy.



    Quma_tb_base defines the common variables, signals and behaviors that are used by
  the child testbenches:
  - variables:
    + spdlogger: console



  - Signals:
    + 200 MHz clock and related global_counter
    + 50 MHz clock and related global_counter



  - Behaviors:
    + Write reset and init signals when simulation starts
    + Set run signal after a duration
    + stop simulation when one of the following happens:
      > The timing queue gets empty. Stop with error.
      > Done signal is set by the QuMA core. Stop with success.
      > The simulation has run for a number of cycles (num_sim_cycles) set by the user.

Usage:
======
    An example can be found in the file tb_cclight.h

*/
#ifndef _TB_BASE_H_
#define _TB_BASE_H_

#include <iomanip>
#include <iostream>
#include <systemc>

#include "global_json.h"
#include "progress_bar.h"
#include "q_data_type.h"

using namespace sc_core;
using namespace sc_dt;

namespace cactus {

class Quma_tb_base : public sc_core::sc_module {
  public:  // clock & reset & init
    sc_in<bool> clock_200MHz;
    sc_in<bool> clock_50MHz;

    sc_signal<bool> reset;
    sc_signal<bool> init;  // related to signal_initiate process, which initiate relevant signals
    sc_signal<bool> started;

  public:
    // classical-specific input
    // interface to the user
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;
    sc_signal<bool>                          Clp2App_done;
    sc_signal<bool>                          Qp2App_eq_empty;

    // quantum-specific input
    sc_signal<bool> run;

  protected:
    void do_test();

  public:
    Progress_bar* progress_bar;

  protected:
    unsigned int m_num_sim_cycles;
    int          m_num_qubits;
    unsigned int m_bar_width = 50;

    void config();

  public:
    Quma_tb_base(const sc_core::sc_module_name& n, unsigned int num_sim_cycles_ = 3000);

    ~Quma_tb_base();

    SC_HAS_PROCESS(Quma_tb_base);
};
}  // namespace cactus

#endif
