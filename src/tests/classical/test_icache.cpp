#include <iostream>
#include <sstream>
#include <systemc>

#include "global_json.h"
#include "icache_rtl.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console       = spdlog::stdout_color_mt("console");
    auto config_logger = spdlog::stdout_color_mt("config_logger");
    console->set_pattern("[%T] [%^%l%$] %v");
    config_logger->set_pattern("[%T] [%^%l%$] %v");

    spdlog::set_level(spdlog::level::debug);
    config_logger->set_level(spdlog::level::info);

    // The following command turns off warning about IEEE 1666 deprecated features.
    // sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test_cache starts.");

    std::string layout_fn("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    std::string elec_config_fn("D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");
    std::string qisa_bin_fn("D:/Projects/QuMA_Sim/test_files/ffdc_latency/prog.bin");

    config_reader global_json;
    global_json.read_from_file(layout_fn);
    global_json.read_electronic_config(elec_config_fn);

    sc_clock                                 clock("my_clock", 1, sc_core::SC_NS, 0.5);
    sc_signal<bool>                          reset;
    sc_signal<bool>                          Clp2Ic_ready;
    sc_signal<bool>                          Clp2Ic_branching;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Ic_target;

    sc_signal<bool>                IC2Clp_valid;
    sc_signal<bool>                IC2Clp_br_done;
    sc_signal<sc_uint<INSN_WIDTH>> IC2Clp_insn;

    console->info("Start instantiating icache...");
    Icache_rtl icache("icache", global_json, qisa_bin_fn);

    console->info("Finished Instantiating icache.");
    icache.clock(clock);
    icache.reset(reset);
    icache.Clp2Ic_ready(Clp2Ic_ready);
    icache.Clp2Ic_branching(Clp2Ic_branching);
    icache.Clp2Ic_target(Clp2Ic_target);
    icache.IC2Clp_valid(IC2Clp_valid);
    icache.IC2Clp_br_done(IC2Clp_br_done);
    icache.IC2Clp_insn(IC2Clp_insn);

    unsigned int cur_insn = 0;

    sc_start(1.0, SC_NS);

    Clp2Ic_ready.write(true);
    Clp2Ic_branching.write(false);
    Clp2Ic_target.write(0);
    console->info("do_test: Successfully wrote initial values to signals.");

    reset.write(true);

    std::stringstream ss;
    console->info("Start reading instructions...");
    for (int i = 0; i < 50; ++i) {
        sc_start(1.0, SC_NS);
        ss.str("");
        ss << "@" << sc_core::sc_time_stamp() << ",";
        Clp2Ic_target.write(i);
        cur_insn = static_cast<unsigned int>(IC2Clp_insn.read());
        console->info("{0:s} current instruction: 0x{1:08X}", ss.str(), cur_insn);
    }
    console->info("Finished reading instructions.");

    sc_stop();

    console->info("test_cache finishes successfully.");

    return 0;
}
