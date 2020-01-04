#ifndef _OP_DECODER_H_
#define _OP_DECODER_H_

#include <string>
#include <systemc>
#include <vector>

#include "generic_if.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "q_data_type.h"
#include "telf_module.h"
#include "util_modules.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Op_decoder : public Telf_module {
  public:
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:
    unsigned int m_num_qubits;

  public:  // methods
    void do_output();

  public:
    void config();

    void add_telf_header();
    void add_telf_line();

    Op_decoder(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Op_decoder);
};

}  // namespace cactus

#endif  //_OP_DECODER_H_
