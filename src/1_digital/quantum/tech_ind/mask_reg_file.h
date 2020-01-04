#ifndef _MASK_REG_FILE_H_
#define _MASK_REG_FILE_H_

#include <string>
#include <systemc>

#include "generic_if.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "q_data_type.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_inout;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_int;
using sc_dt::sc_uint;

class Mask_register_file : public Telf_module {
  public:  // I/O port
    // input
    sc_in<bool>             in_clock;
    sc_in<bool>             reset;
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  private:  // internal register
    Q_mask_reg q_mask_reg;

  public:  // global setting
    unsigned int m_vliw_width;

  public:  // methods:
    void do_write();
    void do_read();

  public:
    void config();

  public:
    void add_telf_header();
    void add_telf_line();

  public:
    Mask_register_file(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Mask_register_file);
};

}  // namespace cactus

#endif  // _MASK_REG_FILE_H_
