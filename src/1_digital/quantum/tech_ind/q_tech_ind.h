#ifndef _Q_TECH_IND_H_
#define _Q_TECH_IND_H_

#include <iterator>
#include <string>
#include <systemc>
#include <vector>

#include "addr_mask_decoder.h"
#include "delay_register.h"
#include "generic_if.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "mask_reg_file.h"
#include "meas_issue_gen.h"
#include "operation_combiner.h"
#include "q_data_type.h"
#include "q_decoder.h"
#include "q_decoder_asm.h"
#include "q_decoder_bin.h"
#include "vliw_pipelane.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_inout;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

class Q_tech_ind : public Telf_module {
  public:  // I/O port
    // in singal
    sc_in<bool> in_clock;
    sc_in<bool> reset;

    sc_in<Qasm_instruction> in_bundle;  // 32bit bin instruction or asm instruction

    sc_in<bool> in_valid_bundle;  // input instruction valid or not

    sc_in<sc_uint<32>> in_rs_wait_time;  // wait time , used for QWAITR

    // out signal
    sc_out<Q_pipe_interface> out_q_pipe_interface;

    // out measurement info
    sc_out<Generic_meas_if> out_Qp2MRF_meas_issue;
    // sc_signal<Generic_meas_if> out_Qp2MRF_meas_issue;

  public:  // signal between modules
    sc_signal<Q_pipe_interface>            q_pipe_interface_sig;
    sc_vector<sc_signal<Q_pipe_interface>> vec_vliw_in_pipe_sig;
    sc_vector<sc_signal<Q_pipe_interface>> vec_vliw_out_pipe_sig;

  public:  // internal modules
    Q_decoder*               q_decoder;
    Operation_combiner       op_combiner;
    sc_vector<Vliw_pipelane> vec_vliw_pipelane;
    Meas_issue_gen           meas_issue_gen;

  public:  // maintain timing info
    timing_type_t tgt_timing_type = TIMING_POINT;
    Timing_info   current_timing;

  public:
    unsigned int     m_num_qubits;
    unsigned int     m_vliw_width;
    Instruction_type m_instruction_type;

  public:
    void config();

    void do_output();  // methods

    Q_tech_ind(const sc_core::sc_module_name& n);

    ~Q_tech_ind();

    SC_HAS_PROCESS(Q_tech_ind);
};

}  // end of namespace cactus

#endif  // _Q_TECH_IND_H_