#ifndef _Q_DECODER_H_
#define _Q_DECODER_H_

#include <string>
#include <systemc>
#include <vector>

#include "generic_if.h"
#include "qasm_instruction.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Q_decoder : public Telf_module {
  public:
    // classical pipeline -> quantum pipeline
    sc_in<bool>             in_clock;
    sc_in<bool>             reset;
    sc_in<Qasm_instruction> in_bundle;
    sc_in<bool>             in_valid_bundle;  // input instruction valid or not
    sc_in<sc_uint<32>>      in_rs_wait_time;  // wait time , used for QWAITR

    // interface to the subsequent unit
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:
    Q_decoder(const sc_core::sc_module_name& n);

    ~Q_decoder();
};

}  // namespace cactus

#endif  // _Q_DECODER_H_
