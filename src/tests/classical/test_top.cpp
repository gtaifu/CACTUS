#include <systemc.h>

#include <iostream>
#include <sstream>

#include "classical_top.h"
#include "global_json.h"
#include "qasm_instruction.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console       = spdlog::stdout_color_mt("console");
    auto config_logger = spdlog::stdout_color_mt("config_logger");
    auto meas_logger   = spdlog::stdout_color_mt("meas_logger");
    auto cache_logger  = spdlog::stdout_color_mt("cache_logger");
    auto clp_logger    = spdlog::stdout_color_mt("clp_logger");

    console->set_pattern("[%T] [%^%l%$] %v");
    config_logger->set_pattern("[%T] [%^%l%$] %v");

    spdlog::set_level(spdlog::level::debug);
    config_logger->set_level(spdlog::level::off);
    meas_logger->set_level(spdlog::level::off);
    cache_logger->set_level(spdlog::level::off);

    // The following command turns off warning about IEEE 1666 deprecated features.
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test_classical_part_top starts.");

    std::string layout_fn("../test_files/hw_config/cclight_config.json");
    std::string qisa_bin_fn("../test_files/classical_insn/classical_insn.bin");
    std::string qisa_asm_fn("../src/tests/classical/test_classical_insn.qisa");

    Global_config& global_config = Global_config::get_instance();
    global_config.read_from_file(layout_fn);
    if (global_config.instruction_type == "binary") {
        global_config.qisa_bin_fn = qisa_bin_fn;
    } else {
        global_config.qisa_bin_fn = qisa_asm_fn;
    }
    global_config.output_dir = "sim_output/";
    // global_json.read_electronic_config(elec_config_fn);

    sc_clock        clock("my_clock", 1, sc_core::SC_NS, 0.5);
    sc_signal<bool> reset;  // in

    // this signal indicates the signal_initiate process, which initiate relevant signals
    sc_signal<bool> init;  // in

    // interface to the user
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> App2Clp_init_pc;  // in
    sc_signal<bool>                          Clp2App_done;     // out

    // instructions sent from classical pipeline to quantum pipeline
    // assisted by handshaking.
    sc_signal<bool>                Qp2Clp_ready;  // in
    sc_signal<bool>                Clp2Qp_valid;  // out
    sc_signal<Qasm_instruction>    Clp2Qp_insn;   // out
    sc_signal<sc_uint<INSN_WIDTH>> Clp2Qp_Rs;     // out

    // interface to quantum pipeline
    sc_signal<bool>                  Qp2MRF_meas_issue;  // in
    sc_vector<sc_signal<sc_uint<1>>> Qp2MRF_qubit_ena;   // in

    // quantum measurement device interface
    sc_vector<sc_signal<sc_uint<1>>> QM2MRF_qubit_ena;   // in
    sc_vector<sc_signal<sc_uint<1>>> QM2MRF_qubit_data;  // in

    console->info("Start instantiating classical part top...");
    Classical_part classical_top("classical_top");
    console->info("Finished instantiating classical part top.");

    unsigned int num_qubits = global_config.num_qubits;

    Qp2MRF_qubit_ena.init(num_qubits);
    QM2MRF_qubit_ena.init(num_qubits);
    QM2MRF_qubit_data.init(num_qubits);

    classical_top.clock(clock);
    classical_top.reset(reset);
    classical_top.init(init);
    classical_top.App2Clp_init_pc(App2Clp_init_pc);
    classical_top.Clp2App_done(Clp2App_done);
    classical_top.Qp2Clp_ready(Qp2Clp_ready);
    classical_top.Clp2Qp_valid(Clp2Qp_valid);
    classical_top.Clp2Qp_insn(Clp2Qp_insn);
    classical_top.Clp2Qp_Rs(Clp2Qp_Rs);
    classical_top.Qp2MRF_meas_issue(Qp2MRF_meas_issue);
    classical_top.Qp2MRF_qubit_ena(Qp2MRF_qubit_ena);
    classical_top.QM2MRF_qubit_ena(QM2MRF_qubit_ena);
    classical_top.QM2MRF_qubit_data(QM2MRF_qubit_data);

    // initial signals
    sc_start(1.0, SC_NS);
    App2Clp_init_pc.write(0);
    Qp2Clp_ready.write(true);
    sc_start(1.0, SC_NS);

    Qp2MRF_meas_issue.write(false);
    for (size_t i = 0; i < num_qubits; ++i) {
        QM2MRF_qubit_ena[i].write(false);
        QM2MRF_qubit_data[i].write(false);
        Qp2MRF_qubit_ena[i].write(false);
    }

    init.write(true);
    sc_start(1.0, SC_NS);
    init.write(false);

    reset.write(true);
    sc_start(1.0, SC_NS);
    reset.write(false);

    console->info("do_test: Successfully wrote initial values to signals.");

    for (size_t i = 0; i < 2000; ++i) {
        sc_start(1.0, SC_NS);
        if (Clp2App_done.read()) {
            sc_stop();
            console->info("test_classical_part_top finishes successfully.");
            return 0;
        }
    }

    sc_stop();

    console->info("test_classical_part_top finishes successfully.");

    return 0;
}
