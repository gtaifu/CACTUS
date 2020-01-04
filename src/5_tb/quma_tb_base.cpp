#include "quma_tb_base.h"

#include <algorithm>

#include "global_counter.h"
#include "global_json.h"
#include "logger_wrapper.h"

namespace cactus {

void Quma_tb_base::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
};

Quma_tb_base::Quma_tb_base(const sc_core::sc_module_name& n, unsigned int num_sim_cycles_) {

    auto logger = get_logger_or_exit("console");

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

    logger->trace("Finished initializing {}...", this->name());
}

Quma_tb_base::~Quma_tb_base() {
    if (progress_bar) {
        delete progress_bar;
        progress_bar = nullptr;
    }
}

void Quma_tb_base::do_test() {

    auto logger = get_logger_or_exit("console");

    auto counter_50MHz = global_counter::get("cycle_counter_50MHz");

    unsigned int cur_cycle;
    bool         is_sc_stop = false;

    std::vector<unsigned int>           vec_percent_cycle;
    std::vector<unsigned int>::iterator it;

    for (unsigned int i = 0; i < m_bar_width; ++i) {
        vec_percent_cycle.push_back(static_cast<unsigned int>(m_num_sim_cycles * i / m_bar_width));
    }

    wait();

    reset.write(1);

    started.write(true);
    App2Clp_init_pc.write(0);
    run.write(0);
    wait();
    reset.write(0);
    wait();
    init.write(1);
    wait();
    init.write(0);

    // wait for 100 cycles to execute starting instructions
    for (int i = 0; i < 100; ++i) {
        wait();
    }

    run.write(1);

    while (true) {
        wait();

        cur_cycle = counter_50MHz->get_cur_cycle_num();

        // If quantum pipeline is over but classical pipeline is still running, then wait until
        //   classical pipeline run to the end

        if (Qp2App_eq_empty.read()) {
            logger->info(
              "{}: The event queue gets empty! (Qp2App_eq_empty=1). "
              "Simulation finished. Current cycle time: {}.",
              this->name(), cur_cycle);

            // 100 % complete
            progress_bar->display_bar(m_num_sim_cycles);

            // print stop info
            std::cout
              << "\n\nCactus has been run all quantum instrcutions, but auxiliary classical "
                 "intructions is still running!"
              << std::endl;

            is_sc_stop = true;
            sc_stop();
        }

        if (Clp2App_done.read()) {
            logger->info(
              "{}: the program has reached its end (Clp2App_done=1). "
              "Wait for dumping timing queue. Current cycle time: {}.",
              this->name(), cur_cycle);

            while (true) {
                wait();

                cur_cycle = counter_50MHz->get_cur_cycle_num();

                if (Qp2App_eq_empty.read()) {
                    logger->info(
                      "{}: the event queue gets empty! (Qp2App_eq_empty=1). "
                      "Simulation finished. Current cycle time: {}.",
                      this->name(), cur_cycle);

                    // 100% complete
                    progress_bar->display_bar(m_num_sim_cycles);

                    // print stop info
                    std::cout << "\n\nCactus has been run all the instrcutions include auxiliary "
                                 "classical instructions and quantum instructions!"
                              << std::endl;

                    is_sc_stop = true;
                    sc_stop();
                }
                if (cur_cycle > m_num_sim_cycles) {
                    logger->info(
                      "{}: Simulation has conducted for {} cycles (50MHz). Simulation stops.",
                      this->name(), cur_cycle);

                    // 100% complete
                    progress_bar->display_bar(m_num_sim_cycles);

                    // print stop info
                    std::cout << "\n\nCactus has reach the end of total " << m_num_sim_cycles
                              << " simulation cycles!" << std::endl;

                    is_sc_stop = true;
                    sc_stop();
                }

                it = std::find(vec_percent_cycle.begin(), vec_percent_cycle.end(), cur_cycle);
                if (it != vec_percent_cycle.end() && !is_sc_stop) {
                    progress_bar->display_bar(cur_cycle);
                }
            }
        }

        // if cur_cycle is in vec_percent_cycle
        it = std::find(vec_percent_cycle.begin(), vec_percent_cycle.end(), cur_cycle);
        if (it != vec_percent_cycle.end() && !is_sc_stop) {
            progress_bar->display_bar(cur_cycle);
        }

        if (cur_cycle > m_num_sim_cycles) {
            logger->info("{}: Simulation has conducted for {} cycles (50MHz). Simulation stops.",
                         this->name(), cur_cycle);

            // 100% complete
            progress_bar->display_bar(m_num_sim_cycles);

            // print stop info
            std::cout << "\n\nCactus has reach the end of total " << m_num_sim_cycles
                      << " simulation cycles!" << std::endl;

            sc_stop();
        }
    }
}

}  // namespace cactus
