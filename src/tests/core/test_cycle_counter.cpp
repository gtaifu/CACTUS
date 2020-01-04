#include <iostream>
#include <sstream>
#include <systemc>

#include "cycle_counter.h"
#include "global_json.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console = spdlog::stdout_color_mt("console");
    console->set_pattern("[%T] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    std::stringstream ss;

    // The following command turns off warning about IEEE 1666 deprecated features.
    // sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test_cycle_counter starts.");

    sc_clock        clock_200MHz("clock_200MHz", 5, sc_core::SC_NS, 0.5);
    sc_signal<bool> started_1;

    console->info("Start instantiating Cycle_counters...");

    // std::shared_ptr<Cycle_counter> cycle_counter_200MHz =
    //     std::make_shared<Cycle_counter>("cycle_counter_200MHz");

    Cycle_counter cycle_counter_200MHz("cycle_counter_200MHz");
    cycle_counter_200MHz.in_clock(clock_200MHz);
    cycle_counter_200MHz.in_started(started_1);

    console->info("Finished instantiating Cycle_counters.");

    sc_start(5.0, SC_NS);
    started_1.write(true);
    sc_start(5.0, SC_NS);
    sc_start(10.0, SC_NS);
    console->info("Counter 200MHz: {}.", cycle_counter_200MHz.get_cur_cycle_num());
    sc_start(10.0, SC_NS);
    console->info("Counter 200MHz: {}.", cycle_counter_200MHz.get_cur_cycle_num());
    sc_start(10.0, SC_NS);
    console->info("Counter 200MHz: {}.", cycle_counter_200MHz.get_cur_cycle_num());
    sc_start(10.0, SC_NS);

    console->info("Counter 200MHz: {}.", cycle_counter_200MHz.get_cur_cycle_num());

    sc_stop();

    console->info("test_cycle_counter finishes successfully.");

    return 0;
}
