#include "counter_registry.h"

#include "logger_wrapper.h"

namespace counter_reg {

counter_registry::counter_registry() {
    auto logger = cactus::get_logger_or_exit("counter_registry_logger");

    logger->debug("counter_registry has been initialized.");
}

bool counter_registry::if_not_exists_(const std::string& counter_name) {

    auto logger = cactus::get_logger_or_exit("counter_registry_logger");

    if (counters_.find(counter_name) != counters_.end()) {

        logger->warn("Counter with name '{}' already exists", counter_name);
        return false;
    }
    return true;
}

std::shared_ptr<cactus::Cycle_counter> counter_registry::get(const std::string& counter_name) {

    auto logger = cactus::get_logger_or_exit("counter_registry_logger");

    logger->debug("Trying to get the counter: {}.", counter_name);

    auto found = counters_.find(counter_name);

    if (found == counters_.end()) {
        logger->error("The cycle counter {} is not found. Simulation aborts", counter_name);
        exit(EXIT_FAILURE);
    }

    return found->second;
}

void counter_registry::register_counter(sc_core::sc_in<bool>&     clock,
                                        sc_core::sc_signal<bool>& started,
                                        std::string               counter_name) {

    auto logger = cactus::get_logger_or_exit("counter_registry_logger");

    std::shared_ptr<cactus::Cycle_counter> new_cycle_counter =
      std::make_shared<cactus::Cycle_counter>(counter_name.c_str());

    new_cycle_counter->in_clock(clock);
    new_cycle_counter->in_started(started);

    if (if_not_exists_(counter_name)) {
        counters_[counter_name] = std::move(new_cycle_counter);
        logger->trace("The counter {} has been registered.", counter_name);
    } else {
        logger->warn("Failed to register the counter {}.", counter_name);
    }
}

}  // end of namespace counter_reg
