#ifndef _MEAS_REG_FILE_TOP_H_
#define _MEAS_REG_FILE_TOP_H_

#include <systemc>

#include "global_json.h"
#include "meas_reg_file_rtl.h"
#include "meas_reg_file_slice.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Meas_reg_file) {
  public:
    sc_in<bool> clock;
    sc_in<bool> reset;

    // interface to classical pipeline
    sc_in<bool>                   Clp2MRF_meas_issue;
    sc_out<bool>                  MRF2Clp_ready;
    sc_vector<sc_out<sc_uint<1>>> MRF2Clp_valid;
    sc_vector<sc_out<sc_uint<1>>> MRF2Clp_data;

    // interface to quantum pipeline
    sc_in<Generic_meas_if> Qp2MRF_meas_issue;
    sc_in<Generic_meas_if> Qp2MRF_meas_cancel;

    // quantum measurement device interface
    sc_in<Generic_meas_if> Qm2MRF_meas_result;

  protected:  // internal modules
    Meas_reg_file_rtl   meas_reg_file_rtl;
    Meas_reg_file_slice meas_reg_file_slice;

  protected:  // internal signals
    sc_signal<bool>                  MCS2MRF_meas_issue_sig;
    sc_signal<bool>                  MRF2MCS_ready_sig;
    sc_vector<sc_signal<sc_uint<1>>> MRF2MCS_valid_sig;
    sc_vector<sc_signal<sc_uint<1>>> MRF2MCS_data_sig;

  protected:
    int num_qubits = 0;

    void config();

  public:
    Meas_reg_file(const sc_core::sc_module_name& n);

};  // end of class Meas_reg_file

}  // end of namespace cactus

#endif  // _MEAS_REG_FILE_TOP_H_
