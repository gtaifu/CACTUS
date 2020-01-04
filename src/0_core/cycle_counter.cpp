#include "cycle_counter.h"

namespace cactus {

SC_HAS_PROCESS(Cycle_counter);

Cycle_counter::Cycle_counter(sc_core::sc_module_name) {

    cur_cycle_num = 0;

    SC_CTHREAD(count, in_clock.pos());
}

void Cycle_counter::count() {

    while (true) {

        wait();

        if (in_started.read()) {
            cur_cycle_num++;
        }
    }
}

unsigned int Cycle_counter::get_cur_cycle_num() { return cur_cycle_num; }
}  // namespace cactus
