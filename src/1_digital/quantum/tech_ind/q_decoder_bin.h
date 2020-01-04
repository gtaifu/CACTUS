#ifndef _Q_DECODER_BIN_H_
#define _Q_DECODER_BIN_H_

#include <string>
#include <systemc>

#include "generic_if.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "q_decoder.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Q_decoder_bin : public Q_decoder {

  public:
    unsigned int m_num_qubits;
    unsigned int m_vliw_width;

  protected:
  public:
    // Write the output. Clocked thread.
    void do_output();
    void log_telf();

    void config();
    void add_telf_header();
    void add_telf_line();

  public:
    Q_decoder_bin(const sc_core::sc_module_name& n);

    ~Q_decoder_bin();

    SC_HAS_PROCESS(Q_decoder_bin);
};

}  // namespace cactus

#endif  // _Q_DECODER_BIN_H_
