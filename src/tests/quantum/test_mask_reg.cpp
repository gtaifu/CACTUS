#include <systemc.h>

#include <iostream>
#include <sstream>

#include "global_json.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "tech_ind/mask_reg_file.h"

using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console = spdlog::stdout_color_mt("console");

    console->set_pattern("[%T] [%^%l%$] %v");

    spdlog::set_level(spdlog::level::debug);

    // The following command turns off warning about IEEE 1666 deprecated features.
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test_mask_reg starts.");

    std::string layout_fn("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    std::string elec_config_fn("D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");

    config_reader global_json;
    global_json.read_from_file(layout_fn);
    global_json.read_electronic_config(elec_config_fn);

    sc_clock        clock_200MHz("clock_200MHz", 1, sc_core::SC_NS, 0.5);
    sc_signal<bool> started_1;

    console->info("Start instantiating Cycle_counters...");
    global_counter::register_counter(clock_200MHz, started_1, "cycle_counter_200MHz");
    console->info("Finished instantiating Cycle_counters.");

    sc_signal<S_OR_T>                   in_s_or_t;
    sc_signal<sc_uint<REG_ADDR_WIDTH>>  in_mask_reg_addr;
    sc_signal<bool>                     in_wr_en;
    sc_signal<sc_uint<1>>               in_wr_s_or_t;
    sc_signal<sc_uint<REG_ADDR_WIDTH>>  in_wr_mask_addr;
    sc_signal<sc_uint<MASK_POS_WIDTH>>  in_wr_mask_pos;
    sc_signal<sc_uint<NUM_MASK_BITS>>   in_wr_mask;
    sc_signal<S_OR_T>                   out_s_or_t;
    sc_signal<sc_uint<ADDR_MASK_WIDTH>> out_mask;

    console->info("Start instantiating mask_reg_file...");
    Mask_register_file mask_reg_file("mask_reg_file", global_json);
    console->info("Finished instantiating mask_reg_file.");

    unsigned int num_qubits = global_json.num_qubits;

    mask_reg_file.in_clock(clock_200MHz);
    mask_reg_file.in_s_or_t(in_s_or_t);
    mask_reg_file.in_mask_reg_addr(in_mask_reg_addr);
    mask_reg_file.in_wr_en(in_wr_en);
    mask_reg_file.in_wr_s_or_t(in_wr_s_or_t);
    mask_reg_file.in_wr_mask_addr(in_wr_mask_addr);
    mask_reg_file.in_wr_mask_pos(in_wr_mask_pos);
    mask_reg_file.in_wr_mask(in_wr_mask);
    mask_reg_file.out_s_or_t(out_s_or_t);
    mask_reg_file.out_mask(out_mask);

    mask_reg_file.open_telf_file();
    started_1.write(true);
    sc_start(1.0, SC_NS);

    sc_start(1.0, SC_NS);

    bool single_or_two = false;
    for (size_t i = 0; i < 32; ++i) {
        sc_start(1.0, SC_NS);
        single_or_two = !single_or_two;

        in_wr_en.write(true);
        in_wr_s_or_t.write(single_or_two);

        if (!single_or_two) {
            in_wr_mask_addr.write(i / 2);
        } else {
            in_wr_mask_addr.write(i);
        }

        in_wr_mask_pos.write(0);
        in_wr_mask.write(i);
    }
    sc_start(1.0, SC_NS);
    in_wr_en.write(false);
    in_s_or_t.write(0);  // single-qubit mask
    for (size_t i = 0; i < 16; ++i) {
        in_mask_reg_addr.write(i);
        sc_start(5.0, SC_NS);
        console->info("out_s_or_t: {}", out_s_or_t.read());
        console->info("i: {}, out_mask: {}", i, static_cast<size_t>(out_mask.read()));
    }

    in_s_or_t.write(1);  // two-qubit mask
    for (size_t i = 0; i < 32; ++i) {
        in_mask_reg_addr.write(i);
        sc_start(5.0, SC_NS);
        console->info("out_s_or_t: {}, addr: {}, out_mask: {}", out_s_or_t.read(), i,
                      static_cast<size_t>(out_mask.read()));
    }

    sc_start(5.0, SC_NS);
    sc_stop();
    mask_reg_file.close_telf_file();
    console->info("test_cache finishes successfully.");

    return 0;
}
