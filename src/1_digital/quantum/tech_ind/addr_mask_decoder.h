#ifndef _ADDR_MASK_DECODER_H_
#define _ADDR_MASK_DECODER_H_

#define ADDR_DECODER_LATENCY 1

#include <systemc>

#include "generic_if.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "q_data_type.h"
#include "telf_module.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

// It converts the mask into a list of qubits or qubit pairs.
class Address_decoder : public Telf_module {
  public:  // i/O port
    // input
    sc_in<bool>             in_clock;
    sc_in<bool>             reset;
    sc_in<Q_pipe_interface> in_q_pipe_interface;

    // output
    sc_out<Q_pipe_interface> out_q_pipe_interface;

  public:
    // The list of all the edges whose right qubit is i.
    std::vector<std::vector<unsigned int>> in_edges_of_qubit;

    // The list of all the edges whose left qubit is i.
    std::vector<std::vector<unsigned int>> out_edges_of_qubit;

  public:
    unsigned int m_num_qubits;

  public:
    void config();

  public:  // methods
    void add_telf_header();
    void add_telf_line();

    void mask_decode();

  public:
    Address_decoder(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Address_decoder);
};
}  // namespace cactus

#endif  // _ADDR_MASK_DECODER_H_
