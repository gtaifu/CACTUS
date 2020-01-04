#ifndef _COUNTER_REGISTRY_H_
#define _COUNTER_REGISTRY_H_

#include "cycle_counter.h"
#include "logger_wrapper.h"

namespace counter_reg {

class counter_registry {

  public:
    // delete the copy constructor
    counter_registry(const counter_registry&) = delete;

    // delete the assignment operator
    counter_registry& operator=(const counter_registry&) = delete;

    static counter_registry& get_instance() {
        // call the priviate constructor to initialize a counter_registry
        // the static one ensures only one copy of the global counter
        static counter_registry s_instance;
        return s_instance;
    }

    // create a new counter using the given signals 'clock', started', and name 'counter_name'
    void register_counter(sc_core::sc_in<bool>& clock, sc_core::sc_signal<bool>& started,
                          std::string counter_name);

    // return the pointer to the counter with the name 'counter_name'
    std::shared_ptr<cactus::Cycle_counter> get(const std::string& counter_name);

  private:
    // priviate constructure, ensure cannot be initialized publicly.
    counter_registry();

    ~counter_registry() = default;

    bool if_not_exists_(const std::string& counter_name);

    std::unordered_map<std::string, std::shared_ptr<cactus::Cycle_counter>> counters_;
};

}  // end of namespace counter_reg

#endif  //_COUNTER_REGISTRY_H_
