/**
 * @file    valid_msmt_mask_gen.h
 * @author  Xiang FU
 * @contact gtaifu@gmail.com
 * @date    07/11/2018
 * @brief
 *   The module Valid_msmt_mask_gen is used to convert the msmt result returned from UHFQA in the
 *   DIO format into <valid, value> pair for each qubit.
 *
 */

#ifndef _VALID_MSMT_MASK_GEN_H_
#define _VALID_MSMT_MASK_GEN_H_

#include <systemc>

#include "global_json.h"

namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

template <int g_num_meas_qubits>
class Valid_msmt_mask_gen : public sc_core::sc_module {
  public:
    sc_in<bool>                           clock;  // general IO
    sc_in<bool>                           reset;
    sc_in<sc_uint<g_num_meas_qubits>>     buffered_mask;  // input
    sc_in<sc_uint<g_num_meas_qubits + 1>> uhfqc2dio;
    sc_out<sc_uint<g_num_meas_qubits>>    uhfqc_result_data;  // output
    sc_out<sc_uint<g_num_meas_qubits>>    uhfqc_result_valid;

    // interface to the FIFO
    sc_in<bool>  mask_buffer_empty;
    sc_out<bool> read_enable;

  protected:  // internal signals
    sc_signal<sc_uint<g_num_meas_qubits>> i_buffered_mask;
    sc_signal<bool>                       i_valid_buf_mask;
    sc_signal<bool>                       i_read_enable;
    sc_signal<sc_uint<2>>                 i_rden_delay;
    sc_signal<bool>                       old_uhfqc_result_valid;
    sc_signal<bool>                       uhfqc_result_valid_re;

  protected:  // internal processes
    inline void buffered_mask_valid();
    inline void read_enable_gen();
    inline void msmt_result_gen();
    inline void rising_edge_detect();
    inline void rising_edge_gen();
    inline void drive_output();

  public:
    SC_CTOR(Valid_msmt_mask_gen) {
        SC_CTHREAD(buffered_mask_valid, clock.pos());
        SC_CTHREAD(read_enable_gen, clock.pos());
        SC_CTHREAD(msmt_result_gen, clock.pos());
        SC_CTHREAD(rising_edge_detect, clock.pos());

        SC_THREAD(rising_edge_gen);
        sensitive << old_uhfqc_result_valid << uhfqc2dio;
        SC_THREAD(drive_output);
        sensitive << i_read_enable;
    }
};

template <int g_num_meas_qubits>
inline void Valid_msmt_mask_gen<g_num_meas_qubits>::buffered_mask_valid() {
    while (true) {
        wait();

        if (reset.read()) {
            i_buffered_mask.write(0);
            i_valid_buf_mask.write(0);
        } else
            i_buffered_mask.write(buffered_mask.read());

        // the mask valid queue has a latency of two cycles
        if (i_rden_delay.read()[1] == 1) i_valid_buf_mask.write(1);

        // a new coming msmt result triggers a new read, which means the current valid mask is
        // used
        if (uhfqc_result_valid_re.read()) i_valid_buf_mask.write(0);
    }
}

template <int g_num_meas_qubits>
inline void Valid_msmt_mask_gen<g_num_meas_qubits>::read_enable_gen() {
    sc_uint<2> v_rden_delay;
    while (true) {
        wait();

        v_rden_delay    = i_rden_delay.read();
        v_rden_delay[1] = v_rden_delay[0];
        v_rden_delay[0] = 0;
        i_rden_delay.write(v_rden_delay);

        i_read_enable.write(0);

        // if there is not a valid msmt mask and the queue is not empty, read one
        if (!i_valid_buf_mask.read() && (i_rden_delay.read() == 0) && !mask_buffer_empty.read()) {
            i_read_enable.write(1);
            i_rden_delay.write(1);
        }

        // if there is a valid msmt mask and a new msmt result comes, read one if the queue is
        // not empty
        if (i_valid_buf_mask.read() && uhfqc_result_valid_re.read() && !mask_buffer_empty.read()) {
            i_read_enable.write(1);
            i_rden_delay.write(1);
        }
    }
}

template <int g_num_meas_qubits>
inline void Valid_msmt_mask_gen<g_num_meas_qubits>::msmt_result_gen() {
    sc_uint<g_num_meas_qubits> v_uhfqcdioin_data;
    while (true) {
        wait();

        uhfqc_result_data.write(0);
        uhfqc_result_valid.write(0);

        v_uhfqcdioin_data = uhfqc2dio.read().range(g_num_meas_qubits - 1, 0);

        if (uhfqc_result_valid_re.read()) {
            uhfqc_result_data.write(v_uhfqcdioin_data & buffered_mask.read());
            uhfqc_result_valid.write(buffered_mask.read());
        }
    }
}

template <int g_num_meas_qubits>
inline void Valid_msmt_mask_gen<g_num_meas_qubits>::rising_edge_detect() {
    while (true) {
        wait();

        old_uhfqc_result_valid.write(uhfqc2dio.read()[g_num_meas_qubits]);
    }
}

template <int g_num_meas_qubits>
inline void Valid_msmt_mask_gen<g_num_meas_qubits>::rising_edge_gen() {
    while (true) {
        wait();

        if (!old_uhfqc_result_valid.read() && uhfqc2dio.read()[g_num_meas_qubits])
            uhfqc_result_valid_re.write(1);
        else
            uhfqc_result_valid_re.write(0);
    }
}

template <int g_num_meas_qubits>
inline void Valid_msmt_mask_gen<g_num_meas_qubits>::drive_output() {
    while (true) {
        wait();

        read_enable.write(i_read_enable.read());
    }
}

}  // namespace cactus

#endif  // _VALID_MSMT_MASK_GEN_H_