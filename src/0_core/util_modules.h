#ifndef _UTIL_MODULES_H_
#define _UTIL_MODULES_H_

#include <systemc>

#include "q_data_type.h"
namespace cactus {
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

// select one of two input operations based on the sel signal as the output operation
// used by the address resolver
SC_MODULE(Operation_mux) {
    sc_in<bool>      in_valid;
    sc_in<Op_sel>    in_sel;
    sc_in<Operation> in_op_left;
    sc_in<Operation> in_op_right;

    sc_out<Operation> out_op;

    void do_work();

    SC_CTOR(Operation_mux) {
        SC_THREAD(do_work);
        sensitive << in_sel << in_op_left << in_op_right << in_valid;
    }
};

// merge two micro-operations using a OR logic. used by the operation_combiner
SC_MODULE(Micro_operation_or) {
  public:
    sc_in<bool>                       in_clock;
    sc_vector<sc_in<Micro_operation>> in_u_ops;
    sc_out<Micro_operation>           out_u_op;

    // Calculate the or result of all input micro-operations,
    // and write the result into the output.
    void do_work();  // synchronous

    SC_CTOR(Micro_operation_or) {
        in_u_ops.init(VLIW_WIDTH);

        SC_THREAD(do_work);
        sensitive << in_u_ops[0] << in_u_ops[1];
    }
};

template <int data_width, int delay_cycles>
SC_MODULE(delay_elements) {
    sc_in<bool>                 clock;
    sc_in<sc_uint<data_width>>  data_in;
    sc_out<sc_uint<data_width>> data_out;

    sc_vector<sc_signal<sc_uint<data_width>>> delay_buffer;

    inline void delay() {
        while (true) {
            wait();

            delay_buffer[0].write(data_in.read());
            for (int i = 0; i < delay_cycles - 1; ++i) {
                delay_buffer[i + 1].write(delay_buffer[i].read());
            }
            data_out.write(delay_buffer[delay_cycles - 1].read());
        }
    }

    SC_CTOR(delay_elements) {
        delay_buffer.init(delay_cycles);

        SC_CTHREAD(delay, clock.pos());
    }
};

}  // namespace cactus

#endif  // _UTIL_MODULES_H_
