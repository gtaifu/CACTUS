#include "meas_reg_file_top.h"

namespace cactus {
void Meas_reg_file::config() {

    Global_config& global_config = Global_config::get_instance();

    num_qubits = global_config.num_qubits;
};

Meas_reg_file::Meas_reg_file(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , meas_reg_file_rtl("meas_reg_file_rtl")
    , meas_reg_file_slice("meas_reg_file_slice") {

    config();

    auto logger = get_logger_or_exit("MRF_logger", CODE_POSITION);

    logger->debug("Start initializing {}...", this->name());

    // port
    MRF2Clp_valid.init(num_qubits);
    MRF2Clp_data.init(num_qubits);

    // signal vector initialization
    MRF2MCS_data_sig.init(num_qubits);
    MRF2MCS_valid_sig.init(num_qubits);

    meas_reg_file_rtl.clock(clock);  // ok
    meas_reg_file_rtl.reset(reset);  // ok

    meas_reg_file_rtl.Qp2MRF_meas_issue(Qp2MRF_meas_issue);    // port  -> to quantum
    meas_reg_file_rtl.Qp2MRF_meas_cancel(Qp2MRF_meas_cancel);  // port  -> to quantum

    meas_reg_file_rtl.Qm2MRF_meas_result(Qm2MRF_meas_result);  // msmt result input

    meas_reg_file_rtl.Clp2MRF_meas_issue(MCS2MRF_meas_issue_sig);  // internal
    meas_reg_file_rtl.MRF2Clp_ready(MRF2MCS_ready_sig);            // internal
    meas_reg_file_rtl.MRF2Clp_data(MRF2MCS_data_sig);              // internal
    meas_reg_file_rtl.MRF2Clp_valid(MRF2MCS_valid_sig);            // internal

    meas_reg_file_slice.clock(clock);                            // ok
    meas_reg_file_slice.reset(reset);                            // ok
    meas_reg_file_slice.Clp2MCS_meas_issue(Clp2MRF_meas_issue);  // port
    meas_reg_file_slice.MCS2Clp_ready(MRF2Clp_ready);            // port
    meas_reg_file_slice.MCS2Clp_valid(MRF2Clp_valid);            // port
    meas_reg_file_slice.MCS2Clp_data(MRF2Clp_data);              // port

    meas_reg_file_slice.MCS2MRF_meas_issue(MCS2MRF_meas_issue_sig);  // internal
    meas_reg_file_slice.MRF2MCS_ready(MRF2MCS_ready_sig);            // internal
    meas_reg_file_slice.MRF2MCS_data(MRF2MCS_data_sig);              // internal
    meas_reg_file_slice.MRF2MCS_valid(MRF2MCS_valid_sig);            // internal

    logger->debug("Finished initializing {}.", this->name());
}
}  // end of namespace cactus
