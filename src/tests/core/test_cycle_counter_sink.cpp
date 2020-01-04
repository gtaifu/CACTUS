#include <iostream>
#include <sstream>
#include <systemc>

#include "cycle_counter.h"
#include "global_counter.h"
#include "global_json.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console = spdlog::stdout_color_mt("console");
    console->set_pattern("[%T] [%n] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    std::stringstream ss;

    // The following command turns off warning about IEEE 1666 deprecated features.
    // sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test_cache starts.");

    sc_clock clock_50MHz("clock_50MHz", 20, sc_core::SC_NS, 0.5);
    sc_clock clock_200MHz("clock_200MHz", 5, sc_core::SC_NS, 0.5);

    std::cout << "clock name: " << clock_200MHz.name() << std::endl;
    sc_signal<bool> started_1;
    sc_signal<bool> started_2;

    console->info("Start instantiating Cycle_counters...");
    global_counter::register_counter(clock_200MHz, started_1, "cycle_counter_200MHz");
    global_counter::register_counter(clock_50MHz, started_2, "cycle_counter_50MHz");

    console->info("Finished instantiating Cycle_counters.");

    sc_start(5.0, SC_NS);
    started_1.write(true);
    sc_start(5.0, SC_NS);
    sc_start(10.0, SC_NS);

    started_2.write(true);
    sc_start(20.0, SC_NS);
    sc_start(10.0, SC_NS);
    sc_start(10.0, SC_NS);
    sc_start(10.0, SC_NS);
    sc_start(10.0, SC_NS);

    auto counter_50MHz  = global_counter::get("cycle_counter_50MHz");
    auto counter_200MHz = global_counter::get("cycle_counter_200MHz");
    console->info("Counter 200MHz: {}, Counter 50MHz: {}.", counter_200MHz->get_cur_cycle_num(),
                  counter_50MHz->get_cur_cycle_num());

    sc_stop();

    console->info("test_cycle_counter finishes successfully.");

    return 0;
}
