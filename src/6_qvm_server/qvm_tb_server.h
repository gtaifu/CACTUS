#ifndef _TB_QVM_H_
#define _TB_QVM_H_

#include <systemc.h>

#include <iomanip>
#include <iostream>
#include <queue>
#include <string>

#include "global_json.h"
#include "logger_wrapper.h"
#include "progress_bar.h"
#include "q_data_type.h"
#include "qvm.h"
#include "socket.h"

using namespace sc_core;
using namespace sc_dt;

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class QVM_Server : public sc_core::sc_module {
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

  public:
    void          launch(int port, int nr_connections);
    Progress_bar* progress_bar;

  protected:  // modules
    QVM           qvm;
    SocketServer* server = NULL;
    Socket*       socket = NULL;

  protected:
    void do_test();
    void receive_cmd();
    void config();
    void execute_cmd();

  protected:
    unsigned int            m_num_sim_cycles;
    int                     m_num_qubits;
    unsigned int            m_bar_width = 50;
    std::queue<std::string> cmd_queue;

  public:
    QVM_Server(const sc_core::sc_module_name& n, unsigned int num_sim_cycles_ = 3000);

    ~QVM_Server();

    SC_HAS_PROCESS(QVM_Server);
};
}  // namespace cactus

#endif
