#include "qvm_tb_server.h"

namespace cactus {

void QVM_Server::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
};

void QVM_Server::receive_cmd() {

    auto logger = get_logger_or_exit("server_logger");
    logger->debug("receive_cmd in progress execute_cmd.");

    auto clock_200MHz = global_counter::get("cycle_counter_200MHz");
    auto cur_cycle    = clock_200MHz->get_cur_cycle_num();

    while (true) {
        std::string cmd = socket->ReceiveLine();

        if (cmd.length() > 0) {
            logger->trace("Cycle {}: Received command: {} (length:{}).", cur_cycle, cmd,
                          cmd.length());
        }

        cmd_queue.push(cmd);

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // if (Clp2App_done.read() == 1) {

    //     socket->SendLine("The program has finished execution.\n");
    // }
}

void QVM_Server::execute_cmd() {
    std::string str_upload_program("upload_program");
    size_t      len_upload_program = str_upload_program.length();

    auto logger = get_logger_or_exit("server_logger");

    if (!cmd_queue.empty()) {

        logger->debug("The cmd queue is not empty. execute one cmd.");

        std::string cmd = cmd_queue.front();
        cmd_queue.pop();

        if (cmd.compare("reset 0") == 0) {
            reset.write(0);
        }
        if (cmd.compare("reset 1") == 0) {
            reset.write(1);
        }

        if (cmd.compare("init 0") == 0) {
            init.write(0);
        }
        if (cmd.compare("init 1") == 0) {
            init.write(1);
        }

        if (cmd.compare("run 0") == 0) {
            run.write(0);
        }
        if (cmd.compare(0, 5, "run 1") == 0) {
            std::cout << "going to run!" << std::endl;
            run.write(1);
        }

        if (cmd.compare("start 0") == 0) {
            started.write(0);
        }
        if (cmd.compare("start 1") == 0) {
            started.write(1);
        }

        if (cmd.compare(0, 4, "stop") == 0) {
            std::cout << "going to stop!" << std::endl;
            sc_stop();
        }

        if (cmd.substr(0, 19).compare(str_upload_program) == 0) {

            auto filename = cmd.substr(len_upload_program, cmd.length() - len_upload_program - 1);
            std::cout << "Program is uploaded via the file: " << filename << std::endl;

            // upload instructions here.
            qvm.init_mem_asm(filename);
        }
    }
}

void QVM_Server::launch(int port, int nr_connections) {

    auto logger = get_logger_or_exit("server_logger");

    server = new SocketServer(port, nr_connections);

    socket = server->Accept();

    logger->info("QVM server has connected to a client.");

    std::thread th(&QVM_Server::receive_cmd, this);
    th.detach();
}

QVM_Server::QVM_Server(const sc_core::sc_module_name& n, unsigned int num_sim_cycles_)
    : qvm("qvm") {
    auto logger = get_logger_or_exit("server_logger");

    logger->trace("Start initializing {}...", this->name());

    global_counter::register_counter(clock_200MHz, started, "cycle_counter_200MHz");
    global_counter::register_counter(clock_50MHz, run, "cycle_counter_50MHz");

    config();

    m_num_sim_cycles = num_sim_cycles_;

    // instance progress bar
    if (m_num_sim_cycles == 0) {
        logger->error("{}: Total simulation cycles is 0. Simulation aborts!", this->name());
        exit(EXIT_FAILURE);
    }

    progress_bar = new Progress_bar(m_num_sim_cycles, m_bar_width);

    SC_CTHREAD(do_test, clock_200MHz.pos());

    qvm.clock(clock_200MHz);
    qvm.clock_50MHz(clock_50MHz);
    qvm.reset(reset);
    qvm.init(init);
    qvm.App2Clp_init_pc(App2Clp_init_pc);
    qvm.Clp2App_done(Clp2App_done);
    qvm.Qp2App_eq_empty(Qp2App_eq_empty);
    qvm.run(run);

    logger->trace("Finished initializing {}...", this->name());
}

QVM_Server::~QVM_Server() {
    if (progress_bar) {
        delete progress_bar;
        progress_bar = nullptr;
    }
}

void QVM_Server::do_test() {

    auto logger = get_logger_or_exit("server_logger");

    auto counter_50MHz  = global_counter::get("cycle_counter_50MHz");
    auto counter_200MHz = global_counter::get("cycle_counter_200MHz");

    std::vector<unsigned int>           vec_percent_cycle;
    std::vector<unsigned int>::iterator it;

    for (unsigned int i = 0; i < m_bar_width; ++i) {
        vec_percent_cycle.push_back(static_cast<unsigned int>(m_num_sim_cycles * i / m_bar_width));
    }

    wait();
    logger->trace("After the first wait.");
    reset.write(1);

    started.write(true);
    App2Clp_init_pc.write(0);
    run.write(0);
    wait();
    logger->trace("After the second wait.");
    reset.write(0);
    wait();
    init.write(1);

    // wait for 100 cycles to execute starting instructions
    // for (int i = 0; i < 1000; ++i) {
    //     wait();
    // }

    bool released = false;
    while (true) {

        wait();

        execute_cmd();

        // std::cout << "the current simulation time: " << sc_time_stamp() << std::endl;
        // std::cout << "clock 200Mhz cycle: " << counter_200MHz->get_cur_cycle_num() << std::endl;
        // std::cout << "(" << sc_time_stamp() << " " << counter_200MHz->get_cur_cycle_num() << " "
        //   << counter_50MHz->get_cur_cycle_num() << " run status:" << run.read() << ") ";

        // if (released == false && init.read() == false) {
        //     std::cout << "Hurray!!! The init signal has been released! " << endl;
        //     released = true;
        // }

        //   if (run.read()) {
        //     std::cout << "Hurray!!! The run signal has been set! " << endl;
        // }
    }
}

}  // namespace cactus
