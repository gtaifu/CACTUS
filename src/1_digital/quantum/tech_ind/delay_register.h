#ifndef _DELAY_REGISTER_H_
#define _DELAY_REGISTER_H_

#include <systemc>

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;

template <int delay_cycles>
SC_MODULE(Delay_register) {
    sc_in<bool>  clock;
    sc_in<bool>  data_in;
    sc_out<bool> data_out;

    sc_vector<sc_signal<bool>> delay_buffer;

    // Total buffered cycles = delay_cycles + 1 since there's one additional clock cycle when write
    //   the output
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

    SC_CTOR(Delay_register) {
        delay_buffer.init(delay_cycles);

        SC_CTHREAD(delay, clock.pos());
    }
};

}  // namespace cactus

#endif  // _DELAY_REGISTER_H_
