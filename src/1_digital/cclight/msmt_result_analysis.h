#ifndef _MSMT_RESULT_ANALYSIS_H_
#define _MSMT_RESULT_ANALYSIS_H_

#include <systemc>

#include "generic_if.h"
#include "global_json.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

#define MEAS_CANCEL_FIFO_DEPTH 8
#define MEAS_RESULT_FIFO_DEPTH 8
//--------------------------------------------------------------------------
// used to match data read/write speed in two timing domain
//--------------------------------------------------------------------------
class Msmt_result_analysis : public Telf_module {
  public:  // general IO
    sc_in<bool> in_clock;
    sc_in<bool> in_50MHz_clock;
    sc_in<bool> reset;

    // input
    // quantum pipeline -> Measurement register file
    sc_in<Generic_meas_if> in_Qp2MRF_meas_cancel;

    // quantum simulator -> Measurement register file
    sc_in<Generic_meas_if> in_meas_result;

    // output
    // quantum pipeline -> Measurement register file
    sc_out<Generic_meas_if> out_Qp2MRF_meas_cancel;

    // quantum simulator -> Measurement register file
    sc_out<Generic_meas_if> out_Qm2MRF_meas_result;

  public:  // internal modules
    // the FIFO used to buffer the meas result and measure cancel signal
    sc_fifo<Generic_meas_if> meas_cancel_fifo;
    sc_fifo<Generic_meas_if> meas_result_fifo;

  public:  // internal signals
    sc_signal<bool> meas_cancel_fifo_full;
    sc_signal<bool> meas_result_fifo_full;

  public:  // internal methods
    // buffered Qp2MRF_meas_cancel signal
    void write_meas_cancel_fifo();
    void read_meas_cancel_fifo();

    // buffered Qm2MRF_meas_result signal
    void write_meas_result_fifo();
    void read_meas_result_fifo();

  public:  // configuration
    unsigned int m_num_qubits = 0;

    void config();

  public:
    Msmt_result_analysis(const sc_core::sc_module_name& n);

    ~Msmt_result_analysis();

    SC_HAS_PROCESS(Msmt_result_analysis);
};

}  // namespace cactus

#endif
