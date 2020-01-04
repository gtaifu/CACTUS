#ifndef _Q_DECODER_ASM_H_
#define _Q_DECODER_ASM_H_

#include <systemc>

#include "generic_if.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "q_decoder.h"
#include "telf_module.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_out;
using sc_dt::sc_uint;

// stimulate
class Q_decoder_asm : public Q_decoder {

  public:
    void config();

    void set_q_insn(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction,
                    const unsigned int& rs_wait);

    void set_smis(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction);

    void set_smit(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction);

    void set_wait(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction,
                  const unsigned int& rs_wait);

    void set_qop(Q_pipe_interface& q_pipe_interface, Qasm_instruction& instruction);

    // no operation
    void set_nop(Q_pipe_interface& q_pipe_interface);

    void add_telf_header();
    void add_telf_line();

  public:  // methods
    void output();

    void log_telf();

  public:
    unsigned int m_num_qubits;
    unsigned int m_vliw_width;

  public:
    Q_decoder_asm(const sc_core::sc_module_name& n);

    ~Q_decoder_asm();

    SC_HAS_PROCESS(Q_decoder_asm);
};

}  // namespace cactus

#endif  // _Q_DECODER_ASM_H_