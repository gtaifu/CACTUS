#include <iostream>
#include <systemc>

#include "global_json.h"
#include "instruction_generator.h"
#include "q_data_type.h"
#include "q_tech_dep.h"
#include "q_tech_ind.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using cactus::config_reader;

using namespace cactus;
using namespace sc_core;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_dt::sc_uint;

void config();

int sc_main(int argc, char* argv[]) {

    auto telf_logger = safe_create_logger("telf_logger", CODE_POSITION);  // used for telf_module

    auto console_logger =
      safe_create_logger("console", CODE_POSITION);  // used for terminal console

    spdlog::set_level(spdlog::level::trace);  // Set global log level to info

    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    config();

    // clock
    sc_clock clock("my_clock", 1, sc_core::SC_NS);

    // instance sub modules
    Instruction_generator insn_stimu("Insn_stimulus");
    Q_tech_ind            q_tech_ind("Q_tech_ind");
    Q_tech_dep            q_tech_dep("Q_tech_dep");

    // signals between modules
    sc_signal<Qasm_instruction> bundle;
    sc_signal<bool>             valid;
    sc_signal<sc_uint<32>>      rs_wait_time;
    sc_signal<Q_pipe_interface> out_tech_ind_q_pipe_interface;
    sc_signal<Generic_meas_if>  meas_issue;

    sc_signal<bool>             run;
    sc_signal<Q_pipe_interface> out_tech_dep_q_pipe_interface;
    sc_signal<bool>             out_eq_empty;
    sc_signal<bool>             out_eq_almostfull;
    sc_signal<bool>             out_Qp2clp_ready;
    sc_signal<Generic_meas_if>  meas_cancel;
    sc_signal<bool>             reset;

    // stimulator
    // ------------------------------------------------------------------------------------------
    // input
    insn_stimu.in_clock(clock);
    insn_stimu.event_queue_almostfull(out_eq_almostfull);

    // interface to q_tech_ind
    insn_stimu.out_bundle(bundle);
    insn_stimu.out_valid(valid);
    insn_stimu.out_rs_wait_time(rs_wait_time);

    // ------------------------------------------------------------------------------------------
    // q_tech_ind
    // ------------------------------------------------------------------------------------------
    // input
    q_tech_ind.in_clock(clock);
    q_tech_ind.in_bundle(bundle);
    q_tech_ind.in_valid_bundle(valid);
    q_tech_ind.in_rs_wait_time(rs_wait_time);
    q_tech_ind.reset(reset);

    // output
    q_tech_ind.out_q_pipe_interface(out_tech_ind_q_pipe_interface);
    q_tech_ind.out_Qp2MRF_meas_issue(meas_issue);

    // ------------------------------------------------------------------------------------------
    // q_tech_dep
    // ------------------------------------------------------------------------------------------
    // input
    q_tech_dep.in_clock(clock);
    q_tech_dep.in_50MHz_clock(clock);
    q_tech_dep.reset(reset);
    q_tech_dep.run(run);
    q_tech_dep.in_q_pipe_interface(out_tech_ind_q_pipe_interface);

    // interface to ADI
    q_tech_dep.out_q_pipe_interface(out_tech_dep_q_pipe_interface);

    // output
    q_tech_dep.out_eq_empty(out_eq_empty);
    q_tech_dep.out_eq_almostfull(out_eq_almostfull);
    q_tech_dep.out_Qp2clp_ready(out_Qp2clp_ready);
    q_tech_dep.out_Qp2MRF_meas_cancel(meas_cancel);

    // run test
    reset.write(false);
    for (unsigned int i = 0; i < 500; ++i) {
        sc_start(1.0, sc_core::SC_NS);
        if (i == 15) {
            run.write(1);
        }
    }

    sc_stop();

    return 0;
}

void config() {

    // The list of all the edges whose right qubit is i.
    std::vector<std::vector<unsigned int>> in_edges_of_qubit;

    // The list of all the edges whose left qubit is i.
    std::vector<std::vector<unsigned int>> out_edges_of_qubit;

    unsigned int left_qubit         = 0;
    unsigned int right_qubit        = 0;
    unsigned int num_directed_edges = 16;

    in_edges_of_qubit.resize(num_directed_edges);
    out_edges_of_qubit.resize(num_directed_edges);
    for (size_t i = 0; i < in_edges_of_qubit.size(); ++i) {
        in_edges_of_qubit[i].clear();
        out_edges_of_qubit[i].clear();
    }

    std::vector<std::pair<int, int>> directed_edges;
    directed_edges.push_back(std::make_pair(2, 0));
    directed_edges.push_back(std::make_pair(0, 3));
    directed_edges.push_back(std::make_pair(3, 1));
    directed_edges.push_back(std::make_pair(1, 4));
    directed_edges.push_back(std::make_pair(2, 5));
    directed_edges.push_back(std::make_pair(5, 3));
    directed_edges.push_back(std::make_pair(3, 6));
    directed_edges.push_back(std::make_pair(6, 4));
    directed_edges.push_back(std::make_pair(0, 2));
    directed_edges.push_back(std::make_pair(3, 0));
    directed_edges.push_back(std::make_pair(1, 3));
    directed_edges.push_back(std::make_pair(4, 1));
    directed_edges.push_back(std::make_pair(5, 2));
    directed_edges.push_back(std::make_pair(3, 5));
    directed_edges.push_back(std::make_pair(6, 3));
    directed_edges.push_back(std::make_pair(4, 6));

    for (size_t i = 0; i < num_directed_edges; i++) {

        left_qubit  = directed_edges[i].first;
        right_qubit = directed_edges[i].second;

        out_edges_of_qubit[left_qubit].push_back(static_cast<unsigned int>(i));
        in_edges_of_qubit[right_qubit].push_back(static_cast<unsigned int>(i));
    }

    Global_config& global_config     = Global_config::get_instance();
    global_config.output_dir         = "./sim_output";
    global_config.num_qubits         = 7;
    global_config.vliw_width         = 2;
    global_config.in_edges_of_qubit  = in_edges_of_qubit;
    global_config.out_edges_of_qubit = out_edges_of_qubit;
    global_config.instruction_type   = Instruction_type::BIN;
    global_config.qisa_asm_fn        = "../src/tests/q_tech_ind/quantum_prog";

    create_dir_if_not_exist(global_config.output_dir);

    if (global_config.instruction_type != Instruction_type::BIN) {
        auto asm_logger =
          safe_create_logger("asm_logger", CODE_POSITION);  // used for terminal console
    }
}