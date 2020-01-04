#include "clock_counter.h"

namespace cactus {

Clock_counter::Clock_counter(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    counter_start.write(true);

    global_counter::register_counter(in_clock, counter_start, "cycle_counter_200MHz");
    global_counter::register_counter(in_50MHz_clock, counter_start, "cycle_counter_50MHz");
}

}  // end of namespace cactus