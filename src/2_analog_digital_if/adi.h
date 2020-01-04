#ifndef _ANALOG_DIGITAL_IF_H_
#define _ANALOG_DIGITAL_IF_H_

#include <systemc>

#include "analog_digital_convert.h"
#include "generic_if.h"
#include "global_json.h"
#include "interface_lib.h"
#include "msmt_result_gen.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Analog_digital_if : public Telf_module {
  public:  // general IO
    sc_in<bool> in_clock;
    sc_in<bool> reset;
    sc_in<bool> in_50MHz_clock;

    // interface between CCL and ADI
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // measurement result
    sc_out<Generic_meas_if> out_meas_result;
    // sc_signal<Generic_meas_if> out_msmt_result;

    // interface between ADI and qubit simulator
    // ADI -> qubit simulator
    sc_out<Ops_2_qsim> ops_2_qsim;

    // qubit simulator -> ADI
    sc_in<Res_from_qsim> msmt_res;

  public:  // internal modules
    Adi_convert*    p_adi_convert = nullptr;
    Msmt_result_gen msmt_result_gen;

  protected:
    unsigned int         m_num_qubits;
    Qubit_simulator_type m_qubit_simulator;

  protected:  // logging methods
    void log_telf();
    void add_telf_header();
    void add_telf_line();

    void config();

    // specified a analog-digital signal convert method
    void instance_adi_convert();

  public:
    Analog_digital_if(const sc_core::sc_module_name& n);

    ~Analog_digital_if();

    SC_HAS_PROCESS(Analog_digital_if);
};

}  // namespace cactus

#endif  // _ANALOG_DIGITAL_IF_H_
