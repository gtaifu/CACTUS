#include <exception>
#include <iostream>
#include <systemc>

#include "cactus_ver.h"
#include "cclight_new.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "q_data_type.h"
#include "tb_qvm.h"

using namespace cactus;

int sc_main(int argc, char* argv[]) {

    // The following command turns off warning about IEEE 1666 deprecated features.
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    Global_config& global_config = Global_config::get_instance();
    global_config.init_cmdparser(argc, argv);
    global_config.run_cmdparser();

    std::cout << "Control Architecture Simulator, " << CACTUS_VERSION << std::endl;
    std::cout << "Simulation Starts." << std::endl;

    sc_clock clock_200MHz("clock_200MHz", 2.0, sc_core::SC_NS, 0.5);
    sc_clock clock_50MHz("clock_50MHz", 20.0, sc_core::SC_NS, 0.5);

    // CC_Light_TB tb("cclight_tb", global_config,  num_sim_cycles);
    QVM_TB tb("qvm_tb", global_config.num_sim_cycles);

    tb.clock_200MHz(clock_200MHz);
    tb.clock_50MHz(clock_50MHz);

    try {
        sc_start();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // dump data memory
    if (!global_config.data_mem_dump_fn.empty()) {
        global_config.data_memory->dump(global_config.data_mem_dump_fn);
    }

    // system("pause");
    return 0;
}
