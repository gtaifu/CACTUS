#ifndef _TB_QVM_H_
#define _TB_QVM_H_

#include <systemc.h>

#include <iomanip>
#include <iostream>

#include "global_json.h"
#include "logger_wrapper.h"
#include "q_data_type.h"
#include "quma_tb_base.h"
#include "qvm.h"

using namespace sc_core;
using namespace sc_dt;

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class QVM_TB : public Quma_tb_base {
  protected:  // modules
    QVM qvm;

  protected:  // signals
              // it seems that we need to add no signals.
  protected:
    // void do_test();

  protected:
    unsigned int m_num_sim_cycles;
    int          m_num_qubits;

    void config();

  public:
    QVM_TB(const sc_core::sc_module_name& n, unsigned int num_sim_cycles_ = 3000);

    SC_HAS_PROCESS(QVM_TB);
};
}  // namespace cactus

#endif
