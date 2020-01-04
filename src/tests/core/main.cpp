#include <iostream>
#include <systemc>

#include "global_json.h"
#include "q_data_type.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using cactus::config_reader;

using namespace sc_core;
using sc_dt::sc_uint;

using cactus::Multi_Q_insn;

SC_MODULE(tb_core) {
    sc_in<bool> clock;

    SC_CTOR(tb_core) { SC_CTHREAD(do_test, clock.pos()); }

    void do_test() {
        wait(2);
        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {

    auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::trace);  // Set global log level to info
    logger->set_pattern("[%T] [%^%l%$] %v");

    // config_reader reader;

    // std::string layout_filename("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    // std::string
    // elec_config_filename("D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");

    // reader.read_from_file(layout_filename);
    // reader.read_electronic_config(elec_config_filename);

    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    sc_clock clock("my_clock", 1, 0.5);

    tb_core tb("tb_core");
    tb.clock(clock);

    // Multi_Q_insn
    Multi_Q_insn insn_a(1, 10);
    Multi_Q_insn insn_b(1, 10);
    Multi_Q_insn insn_c(2, 10);
    Multi_Q_insn insn_d(1000, 10);
    Multi_Q_insn insn_e(1, 10000);

    logger->debug("The width of the Multi_Q_insn: {}.", insn_a.get_width());

    logger->debug("insn_a should be equal to insn_b: {}", insn_a == insn_b);

    std::cout << "insn_a: " << insn_a << std::endl;

    sc_trace_file* wf = sc_create_vcd_trace_file("test_core");

    sc_trace(wf, clock, "clock");
    sc_trace(wf, insn_a, "insn_a");

    sc_start();
    sc_close_vcd_trace_file(wf);

    logger->info("The testbench has finished successfully");

    return 0;
}
