#include <iostream>
#include <systemc>

#include "global_json.h"
#include "instruction_generator.h"
#include "q_data_type.h"
#include "q_decoder.h"
#include "q_decoder_asm.h"
#include "q_decoder_bin.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using cactus::config_reader;

using namespace cactus;
using namespace sc_core;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_dt::sc_uint;

int sc_main(int argc, char* argv[]) {

    auto telf_logger    = safe_create_logger("telf_logger", CODE_POSITION);  // used for telf_module
    auto console_logger = safe_create_logger("console", CODE_POSITION);      // used for telf_module
    auto asm_logger = safe_create_logger("asm_logger", CODE_POSITION);  // used for terminal console
    // auto logger      = spdlog::stdout_color_mt("telf_logger");
    spdlog::set_level(spdlog::level::trace);  // Set global log level to info
    // logger->set_pattern("[%T] [%^%l%$] %v");
    // logger->info("Start Test");

    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    Global_config& global_config   = Global_config::get_instance();
    global_config.instruction_type = Instruction_type::BIN;
    // global_config.output_dir       = "D:/ihub/qarchsim/vsbuild/sim_output/";
    global_config.output_dir = "sim_output/";

    create_dir_if_not_exist(global_config.output_dir);

    sc_clock clock("my_clock", 1, sc_core::SC_NS);

    Q_decoder*            decoder;
    Instruction_generator insn_stimu("Insn_stimulus");

    // instance sub modules
    if (global_config.instruction_type == Instruction_type::BIN) {
        decoder = new Q_decoder_bin("q_decoder_bin");
    } else {
        decoder = new Q_decoder_asm("q_decoder_asm");
    }

    // signals between modules
    sc_signal<Qasm_instruction> bundle;
    sc_signal<bool>             valid;
    sc_signal<sc_uint<32>>      rs_wait_time;
    sc_signal<Q_pipe_interface> q_pipe_interface;
    Q_pipe_interface            out_q_pipe_interface;
    sc_signal<bool>             reset;

    // ------------------------------------------------------------------------------------------
    // stimulate
    // ------------------------------------------------------------------------------------------
    // input
    insn_stimu.in_clock(clock);

    // output
    insn_stimu.out_bundle(bundle);
    insn_stimu.out_valid(valid);
    insn_stimu.out_rs_wait_time(rs_wait_time);

    // ------------------------------------------------------------------------------------------
    // binary instruction decoder
    // ------------------------------------------------------------------------------------------
    // input
    decoder->in_clock(clock);
    decoder->in_bundle(bundle);
    decoder->in_valid_bundle(valid);
    decoder->in_rs_wait_time(rs_wait_time);
    decoder->reset(reset);

    // output
    decoder->out_q_pipe_interface(q_pipe_interface);

    // run test
    reset.write(false);
    for (unsigned int i = 0; i < 15; ++i) {
        sc_start(1.0, sc_core::SC_NS);
    }

    sc_stop();

    return 0;
}