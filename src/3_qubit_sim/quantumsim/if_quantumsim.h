#ifndef _IF_QUANTUMSIM_H_
#define _IF_QUANTUMSIM_H_

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

class If_QuantumSim : public Telf_module {
  public:  // general IO
    sc_in<bool> clock_50MHz;
    sc_in<bool> init;

    // // input
    // sc_vector<sc_in<Quantum_operation>> vec_q_operation;
    // sc_in<bool>                         quantum_triggered;

    // // output
    // sc_vector<sc_out<sc_uint<2>>> vec_msmt_result;

    // input
    sc_in<Ops_2_qsim> ops_2_qsim;  // ADI -> qubit simulator

    // output
    sc_out<Res_from_qsim> msmt_res;  // qubit simulator -> ADI

  protected:
    PyObject* interface;

    void         init_python_api();
    void         apply_quantum_operation();
    void         apply_idle_gate(unsigned int idle_duration, unsigned int qubit);
    void         apply_single_qubit_gate(std::string quantum_operation, unsigned int qubit);
    void         apply_two_qubit_gate(std::string quantum_operation, unsigned int qubit0,
                                      unsigned int qubit1);
    unsigned int measure_qubit(unsigned int qubit);
    void         mock_measure(std::string mock_msmt_res_fn);
    void         print_ptms_to_do(unsigned int qubit);
    void         print_full_dm();

    unsigned int get_idle_duration(bool is_1st_op, unsigned int cur_gate_duration,
                                   unsigned int current_cycle, unsigned int pre_gate_start_point,
                                   unsigned int pre_gate_duration);

    unsigned int starting_cycle = 0;

  protected:  // logging methods
    void add_telf_line();
    void add_telf_header();
    void log_telf();

  protected:  // configurations
    unsigned int     num_qubits       = 0;
    unsigned int     num_msmt_devices = 0;
    unsigned int     cycle_time       = 20;  // ns
    Instruction_type m_instruction_type;

    std::map<std::string, unsigned int> single_qubit_gate_time;
    std::map<std::string, unsigned int> two_qubit_gate_time;
    std::map<std::string, unsigned int> operation_time;
    std::string                         mock_msmt_res_fn;

    // 2D array which store the qubit information for each feedline
    std::vector<std::vector<unsigned int>> qubits_in_each_feedline;

    void config();
    void post_py_process(PyObject* pValue, PyObject* pMethod, const std::string& err_msg);

  public:
    If_QuantumSim(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(If_QuantumSim);
};

}  // namespace cactus

#endif
