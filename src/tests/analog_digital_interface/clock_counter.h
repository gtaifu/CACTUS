#ifndef _CLOCK_COUNTER_H_
#define _CLOCK_COUNTER_H_

#include <systemc>

#include "global_counter.h"
#include "telf_module.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_dt::sc_uint;

class Clock_counter : public Telf_module {
  public:
    sc_in<bool> in_clock;
    sc_in<bool> in_50MHz_clock;

    sc_signal<bool> counter_start;

  public:
    Clock_counter(const sc_core::sc_module_name& n);
};

}  // namespace cactus

#endif  // _CLOCK_COUNTER_H_