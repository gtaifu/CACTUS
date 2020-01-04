#include <systemc.h>

#include <iostream>
#include <sstream>

#include "global_json.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "tech_ind/q_tech_ind.h"

using namespace std;
using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console = spdlog::stdout_color_mt("console");

    console->set_pattern("[%T] [%n] [%^%l%$] %v");

    spdlog::set_level(spdlog::level::info);

    // The following command turns off warning about IEEE 1666 deprecated features.
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test quantum pipeline technology-independent starts.");

    std::string layout_fn("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    std::string elec_config_fn("D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");

    config_reader global_json;
    global_json.read_from_file(layout_fn);
    global_json.read_electronic_config(elec_config_fn);

    sc_clock        clock_200MHz("clock_200MHz", 5.0, sc_core::SC_NS, 0.5);
    sc_signal<bool> started_1;

    console->info("Start instantiating Cycle_counters...");
    global_counter::register_counter(clock_200MHz, started_1, "cycle_counter_200MHz");
    console->info("Finished instantiating Cycle_counters.");

    sc_signal<bool> reset;  // in

    // ----------------------------------------------------------------------------------------
    // input
    // ----------------------------------------------------------------------------------------
    // connected to quantum instruction decoder
    sc_signal<sc_uint<QISA_WIDTH>> in_bundle;
    sc_signal<bool>                in_valid_bundle;
    sc_signal<sc_uint<32>>         in_rs_wait_time;

    // ----------------------------------------------------------------------------------------
    // output
    // ----------------------------------------------------------------------------------------
    sc_vector<sc_signal<Micro_operation>> out_vec_u_ops;
    sc_signal<bool>                       out_valid_timing_point;
    sc_signal<Timing_point>               out_timing_point;
    sc_signal<bool>                       out_Qp2MRF_meas_issue;
    sc_vector<sc_signal<sc_uint<1>>>      out_Qp2MRF_qubit_ena;

    console->info("Start instantiating independent quantum top...");
    Q_tech_ind q_tech_ind("q_tech_ind", global_json);
    console->info("Finished instantiating independent quantum top.");

    unsigned int num_qubits = global_json.num_qubits;

    out_vec_u_ops.init(num_qubits);
    out_Qp2MRF_qubit_ena.init(num_qubits);

    q_tech_ind.in_clock(clock_200MHz);
    q_tech_ind.rst(reset);
    q_tech_ind.in_bundle(in_bundle);
    q_tech_ind.in_valid_bundle(in_valid_bundle);
    q_tech_ind.in_rs_wait_time(in_rs_wait_time);
    q_tech_ind.out_vec_u_ops(out_vec_u_ops);
    q_tech_ind.out_valid_timing_point(out_valid_timing_point);
    q_tech_ind.out_timing_point(out_timing_point);
    q_tech_ind.out_Qp2MRF_meas_issue(out_Qp2MRF_meas_issue);
    q_tech_ind.out_Qp2MRF_qubit_ena(out_Qp2MRF_qubit_ena);

    static const unsigned int arr[] = { 0x40000001, 0x40100002, 0x40200004, 0x40300008, 0x40400010,
                                        0x40500020, 0x40600040, 0x4070007f, 0x40800005, 0x40900068,
                                        0x40a00012, 0x40b00003, 0x50008100, 0x50082000, 0x80000209,
                                        0x80000411, 0x8000f419, 0x80003459, 0x8000f639, 0x8000c001,
                                        0x80010001, 0x80011e0a, 0x80011e0a };

    vector<unsigned int> prog(arr, arr + sizeof(arr) / sizeof(arr[0]));

    sc_start(5.0, SC_NS);
    started_1.write(true);

    for (size_t i = 0; i < prog.size(); ++i) {
        sc_start(5.0, SC_NS);
        in_valid_bundle.write(true);
        in_bundle.write(prog[i]);
        in_rs_wait_time.write(0);
    }
    sc_start(5.0, SC_NS);
    in_valid_bundle.write(false);
    in_bundle.write(0);

    sc_start(50.0, SC_NS);

    sc_stop();

    console->info("quantum pipeline technology-independent finishes successfully.");

    return 0;
}
