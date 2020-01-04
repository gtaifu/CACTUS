#ifndef _VLIW_PIPELANE_H_
#define _VLIW_PIPELANE_H_

#include <string>
#include <systemc>

#include "addr_mask_decoder.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "mask_reg_file.h"
#include "op_decoder.h"
#include "q_data_type.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

/*************** VLIW pipelane ********************/
class Vliw_pipelane : public Telf_module {
  public:  // I/O port
    // input
    sc_in<bool>             in_clock;
    sc_in<bool>             reset;
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:  // signals between modules
    sc_signal<Q_pipe_interface> mask_decode_pipe_sig;
    sc_signal<Q_pipe_interface> register_addressing_pipe_sig;

  public:                                  // internal modules
    Address_decoder    addr_mask_decoder;  // mask decoder
    Mask_register_file mask_reg_file;      // register addressing
    Op_decoder         op_decoder;         // operation decoder

  public:
    void config();

    Vliw_pipelane(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Vliw_pipelane);
};

}  // namespace cactus

#endif  // _VLIW_PIPELANE_H_
