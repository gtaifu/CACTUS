#include "meas_reg_file_slice.h"

#include <sstream>

namespace cactus {

void Meas_reg_file_slice::config() {
    Global_config& global_config = Global_config::get_instance();

    num_qubits = global_config.num_qubits;
};

Meas_reg_file_slice::Meas_reg_file_slice(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n) {

    auto logger = get_logger_or_exit("MRF_logger", CODE_POSITION);

    logger->trace("Start initializing {}.", this->name());

    config();

    MCS2Clp_data.init(num_qubits);
    MCS2Clp_valid.init(num_qubits);
    MRF2MCS_data.init(num_qubits);
    MRF2MCS_valid.init(num_qubits);

    SC_CTHREAD(slice_register, clock.pos());
    SC_CTHREAD(log_IO, clock.pos());
    SC_THREAD(drive_directly);
    sensitive << i_MCS2MRF_meas_issue;

    logger->trace("Finished initializing {}...", this->name());
}

void Meas_reg_file_slice::log_IO() {

    auto logger = get_logger_or_exit("MRF_logger", CODE_POSITION);

    std::stringstream ss;

    while (true) {
        wait();
        ss.str("");
        ss << "@" << sc_core::sc_time_stamp() << ",";
        ss << "Meas_reg_file_slice IO:\n";
        if (Clp2MCS_meas_issue.read())
            ss << "\tClP -> Slice: Clp2MCS_meas_issue: " << Clp2MCS_meas_issue.read() << "\n";
        if (MCS2MRF_meas_issue.read())
            ss << "\tSlice -> MRF: MCS2MRF_meas_issue: " << MCS2MRF_meas_issue.read() << "\n";

        ss << "\n\tMRF -> Slice: MRF2MCS_ready: " << MRF2MCS_ready.read() << "\n";
        ss << "\tMRF result -> MCS: [";
        for (size_t i = 0; i < num_qubits; ++i) {

            ss << i << ":(" << MRF2MCS_valid[i].read() << ", " << MRF2MCS_data[i].read() << ") ";
        }
        ss << "]\n";

        ss << "\n\tMCS -> ClP: MCS2Clp_ready: " << MCS2Clp_ready.read() << "\n";
        ss << "\tMCS result -> ClP: [";
        for (size_t i = 0; i < num_qubits; ++i) {

            ss << i << ":(" << MCS2Clp_valid[i].read() << ", " << MCS2Clp_data[i].read() << ") ";
        }
        ss << "]\n";

        logger->debug("{}: {}", this->name(), ss.str());
    }
}

void Meas_reg_file_slice::slice_register() {
    while (true) {
        wait();

        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
            // valid signal must go low starting from the clock cycle following CLP measure issue
            if (Clp2MCS_meas_issue.read() || i_MCS2MRF_meas_issue.read())
                MCS2Clp_valid[i].write(0);
            else
                MCS2Clp_valid[i].write(MRF2MCS_valid[i].read());
        }

        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
            MCS2Clp_data[i].write(MRF2MCS_data[i].read());
        }

        MCS2Clp_ready.write(MRF2MCS_ready.read());

        i_MCS2MRF_meas_issue.write(Clp2MCS_meas_issue.read());

        if (reset.read()) {
            for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
                MCS2Clp_valid[i].write(1);
            }
            MCS2Clp_ready.write(1);
            i_MCS2MRF_meas_issue.write(0);
        }
    }
}

void Meas_reg_file_slice::drive_directly() {
    while (true) {
        wait();

        MCS2MRF_meas_issue.write(i_MCS2MRF_meas_issue.read());
    }
}
}  // namespace cactus
