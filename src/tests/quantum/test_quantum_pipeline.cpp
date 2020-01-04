#include <systemc.h>

#include <iostream>
#include <sstream>

#include "global_json.h"
#include "quantum_pipeline.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;
using namespace cactus;
using namespace sc_core;

int sc_main(int argc, char* argv[]) {

    auto console = spdlog::stdout_color_mt("console");

    console->set_pattern("[%T] [%n] %^[%L] %v%$");
    // console->set_pattern("[%T] [%n] [%^%L%$] %^%v%$");

    spdlog::set_level(spdlog::level::debug);

    // The following command turns off warning about IEEE 1666 deprecated features.
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    console->info("test quantum pipeline technology-independent starts.");

    std::string layout_fn("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    std::string elec_config_fn("D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");

    config_reader global_json;
    global_json.read_from_file(layout_fn);
    global_json.read_electronic_config(elec_config_fn);

    sc_clock        clock_200MHz("clock_200MHz", 5.0, sc_core::SC_NS, 0.5);
    sc_clock        clock_50MHz("clock_50MHz", 20.0, sc_core::SC_NS, 0.5);
    sc_signal<bool> started_1;
    sc_signal<bool> started_2;

    sc_signal<bool>                  reset;
    sc_signal<bool>                  run;
    sc_vector<sc_signal<sc_uint<4>>> exe_flag;

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
    sc_vector<sc_signal<sc_uint<DIO_WIDTH>>>     vec_mw_DIO;
    sc_signal<sc_uint<64>>                       vsm_DIO;
    sc_vector<sc_signal<sc_uint<DIO_WIDTH>>>     vec_flux_DIO;
    sc_vector<sc_signal<sc_uint<DIO_WIDTH / 2>>> vec_CCL2UHFQA_msmt_DIO;

    // Quantum pipeline tells Classical pipeline if it can further accept new instructions or not
    sc_signal<bool> Qp2Clp_ready;

    // Comprehensive feedback required signals
    sc_signal<bool>                  Qp2MRF_meas_issue;
    sc_vector<sc_signal<sc_uint<1>>> Qp2MRF_qubit_ena;

    console->info("Start instantiating Cycle_counters...");
    global_counter::register_counter(clock_200MHz, started_1, "cycle_counter_200MHz");
    global_counter::register_counter(clock_50MHz, run, "cycle_counter_50MHz");
    console->info("Finished instantiating Cycle_counters.");

    Quantum_pipeline q_pipe("q_pipe", global_json);

    unsigned int num_qubits = global_json.num_qubits;

    exe_flag.init(num_qubits);
    vec_mw_DIO.init(global_json.num_mw_devices);
    vec_flux_DIO.init(global_json.num_flux_devices);
    vec_CCL2UHFQA_msmt_DIO.init(global_json.num_msmt_devices);
    Qp2MRF_qubit_ena.init(num_qubits);

    q_pipe.clock(clock_200MHz);
    q_pipe.clock_50MHz(clock_50MHz);
    q_pipe.reset(reset);
    q_pipe.run(run);
    q_pipe.exe_flag(exe_flag);
    q_pipe.Clp2Qp_valid(in_valid_bundle);
    q_pipe.Clp2Qp_insn(in_bundle);
    q_pipe.Clp2Qp_Rs(in_rs_wait_time);
    q_pipe.vec_mw_DIO(vec_mw_DIO);
    q_pipe.vsm_DIO(vsm_DIO);
    q_pipe.vec_flux_DIO(vec_flux_DIO);
    q_pipe.vec_CCL2UHFQA_msmt_DIO(vec_CCL2UHFQA_msmt_DIO);
    q_pipe.Qp2Clp_ready(Qp2Clp_ready);
    q_pipe.Qp2MRF_meas_issue(Qp2MRF_meas_issue);
    q_pipe.Qp2MRF_qubit_ena(Qp2MRF_qubit_ena);

    static const unsigned int arr[] = { 0x40000001, 0x40100002, 0x40200004, 0x40300008, 0x40400010,
                                        0x40500020, 0x40600040, 0x4070007f, 0x40800005, 0x40900068,
                                        0x40a00012, 0x40b00003, 0x50008100, 0x50082000, 0x80000209,
                                        0x80000411, 0x8000f419, 0x80003459, 0x8000f639, 0x8000c001,
                                        0x80010001, 0x80011e0a, 0x80011e0a };

    vector<unsigned int> prog(arr, arr + sizeof(arr) / sizeof(arr[0]));

    sc_start(5.0, SC_NS);

    started_1.write(true);

    for (int i = 0; i < num_qubits; ++i) {
        exe_flag[i].write(0xf);
    }

    for (size_t i = 0; i < prog.size(); ++i) {
        sc_start(5.0, SC_NS);
        in_valid_bundle.write(true);
        in_bundle.write(prog[i]);
        in_rs_wait_time.write(0);
    }
    run.write(true);
    sc_start(5.0, SC_NS);
    in_valid_bundle.write(false);
    in_bundle.write(0);

    sc_start(2000.0, SC_NS);

    auto counter_50MHz = global_counter::get("cycle_counter_50MHz");
    console->info("Simulation ends at cycle (50MHz clock): {}", counter_50MHz->get_cur_cycle_num());
    sc_stop();

    console->info("quantum pipeline technology-independent finishes successfully.");

    return 0;
}
