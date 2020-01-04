#include <systemc.h>

#include <iostream>
#include <sstream>

#include "global_json.h"
#include "meas_reg_file_top.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace cactus;
using namespace sc_core;

void print_msmt_state(unsigned int num_qubits, sc_vector<sc_signal<sc_uint<1>>>& MRF2Clp_valid,
                      sc_vector<sc_signal<sc_uint<1>>>& MRF2Clp_data) {

    auto console = spdlog::get("console");

    std::stringstream ss;
    ss.str("");
    for (size_t i = 0; i < num_qubits; ++i) {
        ss << "\tqubit " << i << ", valid: " << MRF2Clp_valid[i].read()
           << ", value: " << MRF2Clp_data[i].read() << std::endl;
    }

    console->info("Qubit state: \n{}", ss.str());
}

int sc_main(int argc, char* argv[]) {

    std::stringstream ss;
    auto              console       = spdlog::stdout_color_mt("console");
    auto              config_logger = spdlog::stdout_color_mt("config_logger");
    console->set_pattern("[%T] [%^%l%$] %v");
    config_logger->set_pattern("[%T] [%^%l%$] %v");

    spdlog::set_level(spdlog::level::debug);
    config_logger->set_level(spdlog::level::off);

    // The following command turns off warning about IEEE 1666 deprecated features.
    // sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test_cache starts.");

    std::string layout_fn("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    std::string elec_config_fn("D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");
    std::string qisa_bin_fn("D:/Projects/QuMA_Sim/test_files/ffdc_latency/prog.bin");

    config_reader global_json;
    global_json.read_from_file(layout_fn);
    global_json.read_electronic_config(elec_config_fn);

    unsigned int num_qubits = global_json.num_qubits;

    sc_clock clock("my_clock", 1, sc_core::SC_NS, 0.5);

    sc_signal<bool>                  reset;
    sc_signal<bool>                  Clp2MRF_meas_issue;  // input
    sc_signal<bool>                  MRF2Clp_ready;
    sc_vector<sc_signal<sc_uint<1>>> MRF2Clp_valid;
    sc_vector<sc_signal<sc_uint<1>>> MRF2Clp_data;

    // interface to quantum pipeline
    sc_signal<bool>                  Qp2MRF_meas_issue;  // input
    sc_vector<sc_signal<sc_uint<1>>> Qp2MRF_qubit_ena;   // input

    // quantum measurement device interface
    sc_vector<sc_signal<sc_uint<1>>> QM2MRF_qubit_ena;   // input
    sc_vector<sc_signal<sc_uint<1>>> QM2MRF_qubit_data;  // input

    MRF2Clp_valid.init(num_qubits);
    MRF2Clp_data.init(num_qubits);
    Qp2MRF_qubit_ena.init(num_qubits);
    QM2MRF_qubit_ena.init(num_qubits);
    QM2MRF_qubit_data.init(num_qubits);

    console->info("Start instantiating meas_reg_file...");
    Meas_reg_file meas_reg_file("meas_reg_file", global_json);
    console->info("Finished Instantiating meas_reg_file.");

    meas_reg_file.clock(clock);
    meas_reg_file.reset(reset);
    meas_reg_file.Clp2MRF_meas_issue(Clp2MRF_meas_issue);
    meas_reg_file.Qp2MRF_meas_issue(Qp2MRF_meas_issue);
    meas_reg_file.Qp2MRF_qubit_ena(Qp2MRF_qubit_ena);
    meas_reg_file.QM2MRF_qubit_ena(QM2MRF_qubit_ena);
    meas_reg_file.QM2MRF_qubit_data(QM2MRF_qubit_data);

    meas_reg_file.MRF2Clp_ready(MRF2Clp_ready);
    meas_reg_file.MRF2Clp_valid(MRF2Clp_valid);
    meas_reg_file.MRF2Clp_data(MRF2Clp_data);

    sc_start(1.0, SC_NS);

    reset.write(true);
    sc_start(1.0, SC_NS);
    reset.write(false);
    console->info("Release the reset signal.");

    console->info("Qubit state after initialization...");
    print_msmt_state(num_qubits, MRF2Clp_valid, MRF2Clp_data);

    Clp2MRF_meas_issue.write(true);
    ss.str("");
    ss << "@" << sc_core::sc_time_stamp() << ",";
    console->debug("{}: Clp2MRF_meas_issue: {}", ss.str(), Clp2MRF_meas_issue.read());

    sc_start(1.0, SC_NS);
    Clp2MRF_meas_issue.write(false);
    console->info("A measure issue from the classical pipieline...");
    print_msmt_state(num_qubits, MRF2Clp_valid, MRF2Clp_data);

    sc_start(1.0, SC_NS);

    sc_start(2.0, SC_NS);
    for (size_t i = 0; i < num_qubits; ++i) {
        Qp2MRF_qubit_ena[i].write(true);
    }
    Qp2MRF_meas_issue.write(true);
    sc_start(1.0, SC_NS);
    Qp2MRF_meas_issue.write(false);
    for (size_t i = 0; i < num_qubits; ++i) {
        Qp2MRF_qubit_ena[i].write(false);
    }
    sc_start(1.0, SC_NS);
    console->info("The quantum pipeline confirms that all qubits are measured...");
    print_msmt_state(num_qubits, MRF2Clp_valid, MRF2Clp_data);

    sc_start(1.0, SC_NS);
    for (size_t i = 0; i < num_qubits; ++i) {
        QM2MRF_qubit_ena[i].write(true);
        QM2MRF_qubit_data[i].write(i % 2);
    }
    sc_start(1.0, SC_NS);
    for (size_t i = 0; i < num_qubits; ++i) {
        QM2MRF_qubit_ena[i].write(false);
        QM2MRF_qubit_data[i].write(0);
    }
    sc_start(5.0, SC_NS);
    console->info("Measurement result of all qubits are ready:");
    print_msmt_state(num_qubits, MRF2Clp_valid, MRF2Clp_data);

    // unsigned int cur_insn = 0;
    // console->info("Start reading instructions...");
    // for (int i = 0; i < 40; ++i)
    // {
    //     sc_start(1.0, SC_NS);
    //     ss.str("");
    //     ss << "@" << sc_core::sc_time_stamp() << ",";
    //     cur_insn = static_cast<unsigned int>(IC2Clp_insn.read());
    //     console->info("{0:s} current instruction: 0x{1:08X}", ss.str(), cur_insn);
    // }

    // sc_start(1.0, SC_NS);
    // Clp2Ic_target.write(i);
    // Clp2Ic_branching.write(true);
    // sc_start(1.0, SC_NS);

    // for (int i = 0; i < 50; ++i)
    // {
    //     sc_start(1.0, SC_NS);
    //     ss.str("");
    //     ss << "@" << sc_core::sc_time_stamp() << ",";
    //     cur_insn = static_cast<unsigned int>(IC2Clp_insn.read());
    //     console->info("{0:s} current instruction: 0x{1:08X}", ss.str(), cur_insn);
    // }

    // console->info("Finished reading instructions.");

    sc_stop();

    console->info("test_cache finishes successfully.");

    return 0;
}
