#ifndef _MEAS_CLP_SLICE_H_
#define _MEAS_CLP_SLICE_H_

#include <systemc>

#include "global_json.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Meas_reg_file_slice) {
  public:
    sc_in<bool> clock;
    sc_in<bool> reset;

    // classical pipeline interface
    sc_in<bool>  Clp2MCS_meas_issue;
    sc_out<bool> MCS2MRF_meas_issue;

    sc_out<bool>                  MCS2Clp_ready;
    sc_vector<sc_out<sc_uint<1>>> MCS2Clp_valid;
    sc_vector<sc_out<sc_uint<1>>> MCS2Clp_data;

    sc_in<bool>                  MRF2MCS_ready;
    sc_vector<sc_in<sc_uint<1>>> MRF2MCS_data;
    sc_vector<sc_in<sc_uint<1>>> MRF2MCS_valid;

  protected:  // modules
    void log_IO();

    void slice_register();
    void drive_directly();

  protected:  // signals
    sc_signal<bool> i_MCS2MRF_meas_issue;

  protected:
    unsigned int num_qubits = 0;

    void config();

  public:
    Meas_reg_file_slice(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Meas_reg_file_slice);
};
}  // namespace cactus

#endif
