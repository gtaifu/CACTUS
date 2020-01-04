#include <exception>
#include <iostream>
#include <systemc>

#include "cactus_ver.h"
#include "cclight_new.h"
#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"
#include "q_data_type.h"
#include "qvm_tb_server.h"

using namespace cactus;

int sc_main(int argc, char* argv[]) {

    // The following command turns off warning about IEEE 1666 deprecated features.
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);

    Global_config& global_config = Global_config::get_instance();

    char* c1 = "-c";
    char* c2 = "D:\\GitHub\\git_pcl\\Cactus\\test_files\\test_input_file_list.json";

    char** new_argv = new char*[3];
    new_argv[0]     = argv[0];
    new_argv[1]     = c1;
    new_argv[2]     = c2;
    argc            = 3;

    global_config.init_cmdparser(argc, new_argv);
    global_config.run_cmdparser();

    std::cout << "CACTUS Server " << CACTUS_VERSION << " starts." << std::endl;

    std::cout << "Time resolution: " << sc_get_time_resolution() << std::endl;
    sc_clock clock_200MHz("clock_200MHz", 5.0, sc_core::SC_NS, 0.5);
    sc_clock clock_50MHz("clock_50MHz", 20.0, sc_core::SC_NS, 0.5);

    // CC_Light_TB tb("cclight_tb", global_config,  num_sim_cycles);
    QVM_Server cactus_server("cactus_server", global_config.num_sim_cycles);

    cactus_server.clock_200MHz(clock_200MHz);
    cactus_server.clock_50MHz(clock_50MHz);

    int port           = 5000;
    int nr_connections = 5;
    cactus_server.launch(port, nr_connections);

    std::cout << "finished launching the server. Waiting for client connection." << std::endl;
    try {
        sc_start();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    std::cout << "finished starting systemc." << std::endl;

    // dump data memory
    if (!global_config.data_mem_dump_fn.empty()) {
        global_config.data_memory->dump(global_config.data_mem_dump_fn);
    }

    // system("pause");
    return 0;
}
