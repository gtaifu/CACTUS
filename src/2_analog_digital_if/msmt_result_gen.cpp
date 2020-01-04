#include "msmt_result_gen.h"

namespace cactus {

void Msmt_result_gen::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "meas_result_gen");
}

Msmt_result_gen::Msmt_result_gen(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    open_telf_file();

    SC_CTHREAD(gen_msmt_result, in_50MHz_clock);

    SC_CTHREAD(log_telf, in_50MHz_clock);

    logger->trace("Finished initializing {}...", this->name());
}

void Msmt_result_gen::gen_msmt_result() {

    auto logger = get_logger_or_exit("console");

    Generic_meas_if   meas_result;
    std::vector<bool> vec_meas_data;
    std::vector<bool> vec_meas_data_valid;

    Res_from_qsim                                      i_msmt_res;
    std::vector<std::pair<unsigned int, unsigned int>> results;

    while (true) {
        wait();

        // read result from simulator
        i_msmt_res = msmt_res.read();
        results    = i_msmt_res.results;

        // default: no returned measurement results.
        meas_result.reset();
        vec_meas_data.clear();
        vec_meas_data_valid.clear();
        for (size_t i = 0; i < m_num_qubits; i++) {
            vec_meas_data.push_back(false);
            vec_meas_data_valid.push_back(false);
        }

        // check whether result size exceed number of qubits
        if (results.size() > m_num_qubits) {
            logger->error(
              "{}: Qubit simulator has returned more than '{}' results which is more than the "
              "total qubit number. Simulation aborts!",
              this->name(), results.size());
            exit(EXIT_FAILURE);
        }

        if (!reset.read()) {
            for (size_t i = 0; i < results.size(); i++) {
                unsigned int qubit  = results[i].first;
                unsigned int result = results[i].second;

                // result is 0 or 1
                vec_meas_data[qubit]       = (result > 0);
                vec_meas_data_valid[qubit] = true;
            }
        }

        meas_result.set_meas_data(vec_meas_data);
        meas_result.set_meas_data_valid(vec_meas_data_valid);

        // output measurement result
        out_meas_result.write(meas_result);
    }
}

void Msmt_result_gen::log_telf() {

    Generic_meas_if   meas_result;
    std::vector<bool> vec_meas_data;
    std::vector<bool> vec_meas_data_valid;
    bool              meas_result_valid;

    while (true) {

        wait();

        if (is_telf_on) {

            meas_result         = out_meas_result.read();
            vec_meas_data       = meas_result.get_meas_data();
            vec_meas_data_valid = meas_result.get_meas_data_valid();

            meas_result_valid = 0;
            for (size_t i = 0; i < vec_meas_data_valid.size(); ++i) {
                meas_result_valid |= vec_meas_data_valid[i];
            }

            if (meas_result_valid) {
                // telf_os << meas;
                telf_os << std::setfill(' ') << std::setw(16)
                        << sc_core::sc_time_stamp().to_string();
                for (size_t i = 0; i < vec_meas_data.size(); ++i) {
                    if (vec_meas_data_valid[i]) {
                        telf_os << " " << std::setfill(' ') << std::setw(3) << vec_meas_data[i];
                    } else {
                        telf_os << " " << std::setfill(' ') << std::setw(3) << "x";
                    }
                }
                telf_os << std::endl;
            }
        }
    }
}

Msmt_result_gen::~Msmt_result_gen() { close_telf_file(); }

void Msmt_result_gen::add_telf_header() {
    telf_os << std::setfill(' ') << std::setw(16) << "sim_time";
    for (unsigned int i = 0; i < m_num_qubits; ++i) {
        std::string qubit = "q" + to_string(i);
        telf_os << " " << std::setfill(' ') << std::setw(3) << qubit;
    }
    telf_os << std::endl;
}

void Msmt_result_gen::add_telf_line() {}

}  // namespace cactus
