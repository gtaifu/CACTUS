#ifndef _ADI_CONVERT_H_
#define _ADI_CONVERT_H_

#include <map>
#include <systemc>

#include "generic_if.h"
#include "global_json.h"
#include "interface_lib.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Adi_convert : public Telf_module {
  public:  // general IO
    sc_in<bool> in_clock;
    sc_in<bool> reset;
    sc_in<bool> in_50MHz_clock;

    // interface between CCL and ADI
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // interface between ADI and qubit simulator
    sc_out<Ops_2_qsim> ops_2_qsim;  // ADI -> qubit simulator

  public:
    unsigned int                    m_num_qubits;
    std::map<uint64_t, std::string> opcode_to_opname_lut_content;

  public:
    void config();
    void set_opcode_to_opname_lut(std::map<uint64_t, std::string> lut_content);

  public:  // virtual methods
    virtual void signal_convert();

  public:
    Adi_convert(const sc_core::sc_module_name& n);

    ~Adi_convert();
};

class Adi_convert_to_quantumsim : public Adi_convert {
  public:  // methods
    void signal_convert();

  public:
    Adi_convert_to_quantumsim(const sc_core::sc_module_name& n);

    ~Adi_convert_to_quantumsim();

    SC_HAS_PROCESS(Adi_convert_to_quantumsim);
};

class Adi_convert_to_circuit : public Adi_convert {
  public:  // methods
    void signal_convert();

  public:
    Adi_convert_to_circuit(const sc_core::sc_module_name& n);

    ~Adi_convert_to_circuit();

    SC_HAS_PROCESS(Adi_convert_to_circuit);
};

}  // namespace cactus

#endif  // _ADI_CONVERT_H_
