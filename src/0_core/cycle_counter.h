#ifndef _CYCLE_COUNTER_H_
#define _CYCLE_COUNTER_H_

#include <iostream>
#include <systemc>
#include <unordered_map>

#include "logger_wrapper.h"

namespace cactus {

SC_MODULE(Cycle_counter) {
  public:
    sc_core::sc_in<bool> in_clock;
    sc_core::sc_in<bool> in_started;

  public:
    unsigned int get_cur_cycle_num();

  protected:
    unsigned int cur_cycle_num = 0;

    void count();

  public:
    SC_CTOR(Cycle_counter);
};

}  // end of namespace cactus

#endif  // _CYCLE_COUNTER_H_
