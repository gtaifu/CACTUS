#ifndef _QVM_H_
#define _QVM_H_

#include <string>
#include <systemc>

#include "adi.h"
#include "cclight_new.h"
#include "generic_if.h"
#include "if_QIcircuit.h"
#include "if_quantumsim.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(QVM) {
  public:  // IO ports
    // general input
    sc_in<bool> clock;
    sc_in<bool> clock_50MHz;
    sc_in<bool> reset;
    sc_in<bool> init;

    // classical pipeline interface to the user
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;
    sc_out<bool>                         Clp2App_done;
    sc_out<bool>                         Qp2App_eq_empty;

    // quantum-specific input from the user
    sc_in<bool> run;

  public:
    void init_mem_asm(std::string asm_fn);

  private:  // internal modules
    CC_Light          cclight;
    Analog_digital_if adi;
    If_QuantumSim*    p_quantumsim;
    If_QIcircuit*     p_QIcircuit;

  private:  // internal signals
    // interface between digital part (cclight) and ADI
    sc_signal<Q_pipe_interface> q_pipe_interface_sig;
    sc_signal<Generic_meas_if>  meas_result_sig;

    // interface between ADI and qubit simulator
    sc_signal<Ops_2_qsim>    ops_2_qsim;  // ADI -> qubit simulator
    sc_signal<Res_from_qsim> msmt_res;    // qubit simulator -> ADI

  protected:
    unsigned int         m_num_qubits;
    Qubit_simulator_type m_qubit_simulator;

    void config();

  public:
    QVM(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(QVM);
};

}  // namespace cactus

#endif  //_QVM_H_
