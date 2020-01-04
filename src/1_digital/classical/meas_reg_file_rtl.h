#ifndef _MEAS_REG_FILE_H_
#define _MEAS_REG_FILE_H_

#include <systemc>

#include "generic_if.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "num_util.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Meas_reg_file_rtl) {
  public:
    std::ofstream msmt_result_veri_out;

    sc_in<bool> clock;
    sc_in<bool> reset;

    // quantum pipeline interface
    sc_in<Generic_meas_if> Qp2MRF_meas_issue;
    sc_in<Generic_meas_if> Qp2MRF_meas_cancel;

    // quantum measurement device interface
    sc_in<Generic_meas_if> Qm2MRF_meas_result;

    // classical pipeline interface
    sc_in<bool>                   Clp2MRF_meas_issue;
    sc_vector<sc_out<sc_uint<1>>> MRF2Clp_data;
    sc_vector<sc_out<sc_uint<1>>> MRF2Clp_valid;
    sc_out<bool>                  MRF2Clp_ready;

  protected:  // internal signals
    // this signal counts the number of measure instructions that have been issued by the classical
    // pipeline, but have not yet been decoded by the microcode unit.
    sc_signal<sc_uint<G_INTERLOCK_COUNTER_BITS>> i_lock_counter;
    sc_signal<bool>                              i_lock_ready;
    sc_signal<bool>                              i_lock_threshold;
    sc_vector<sc_signal<sc_uint<1>>>             qubit_valid;
    sc_vector<sc_signal<sc_uint<1>>>             qubit_threshold;
    sc_vector<sc_signal<sc_uint<1>>>             qubit_data;
    // counter for each qubit
    sc_vector<sc_signal<sc_uint<G_INTERLOCK_COUNTER_BITS>>> pending_meas_counter;

    // signal derived by quantum pipeline interface
    sc_signal<bool>                  Qp2MRF_meas_issue_sig;
    sc_vector<sc_signal<sc_uint<1>>> Qp2MRF_qubit_ena_sig;
    sc_vector<sc_signal<sc_uint<1>>> Qp2MRF_qubit_ena_cancel_sig;

    // signal derived by quantum measurement device sig
    sc_vector<sc_signal<sc_uint<1>>> Qm2MRF_qubit_ena_sig;
    sc_vector<sc_signal<sc_uint<1>>> Qm2MRF_qubit_data_sig;

    int num_cycles = 0;

  protected:  // modules
    void log_IO();

    void update_signals();
    void interlocking_counter();
    void interlocking_ready();
    void qubit_valid_counter();
    void generate_signals();
    void output_register();

    void clock_counter();
    void write_output_file();

    void open_telf_file();

    void close_telf_file();

  protected:
    unsigned int num_qubits = 0;
    std::string  m_output_dir;

    void config();

  public:
    Meas_reg_file_rtl(const sc_core::sc_module_name& n);

    ~Meas_reg_file_rtl();

    SC_HAS_PROCESS(Meas_reg_file_rtl);
};
}  // namespace cactus

#endif
