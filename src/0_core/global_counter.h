#ifndef _GLOBAL_COUNTER_H_
#define _GLOBAL_COUNTER_H_

#include "counter_registry.h"

namespace global_counter {

inline void register_counter(sc_core::sc_in<bool>& clock, sc_core::sc_signal<bool>& started,
                             std::string counter_name) {
    counter_reg::counter_registry::get_instance().register_counter(clock, started, counter_name);
}

inline std::shared_ptr<cactus::Cycle_counter> get(const std::string& counter_name) {
    return counter_reg::counter_registry::get_instance().get(counter_name);
}

}  // end of namespace global_counter

#endif  // _GLOBAL_COUNTER_H_
