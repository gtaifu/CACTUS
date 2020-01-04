#ifndef _IF_QICIRCUIT_H_
#define _IF_QICIRCUIT_H_

#include <systemc.h>

#include <map>
#include <string>
#include <vector>

#include "global_json.h"
#include "interface_lib.h"
#include "telf_module.h"

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;
using std::vector;

class If_QIcircuit : public Telf_module {
  public:  // general IO
    sc_in<bool> clock_50MHz;
    sc_in<bool> init;

    // input
    sc_in<Ops_2_qsim> ops_2_qsim;  // ADI -> qubit simulator

    // output
    sc_out<Res_from_qsim> msmt_res;  // qubit simulator -> ADI

  protected:
    PyObject* interface;

    void         init_python_api();
    void         apply_quantum_operation();
    void         apply_single_qubit_gate(std::string quantum_operation, unsigned int qubit);
    void         apply_two_qubit_gate(std::string quantum_operation, unsigned int qubit0,
                                      unsigned int qubit1);
    void         measure_qubit(unsigned int qubit);
    unsigned int run_circuit(unsigned int num_measure);

    unsigned int starting_cycle = 0;
    unsigned int measure_num    = 0;  // record # of  measurement operations

  protected:  // logging methods
    void add_telf_line();
    void add_telf_header();
    void log_telf();

  protected:  // configurations
    unsigned int                        num_qubits       = 0;
    unsigned int                        num_msmt_devices = 0;
    unsigned int                        cycle_time       = 20;  // ns
    unsigned int                        num_measure      = 0;
    std::map<std::string, unsigned int> single_qubit_gate_time;
    std::map<std::string, unsigned int> two_qubit_gate_time;
    std::map<std::string, unsigned int> operation_time;

    void config();
    void post_py_process(PyObject* pValue, PyObject* pMethod, const std::string& err_msg);

  public:
    If_QIcircuit(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(If_QIcircuit);
};

}  // namespace cactus

#endif
