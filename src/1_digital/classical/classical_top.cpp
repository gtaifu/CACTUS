#include "classical_top.h"

namespace cactus {

void Classical_part::config() {

    Global_config& global_config = Global_config::get_instance();

    num_qubits = global_config.num_qubits;
};

void Classical_part::init_mem_asm(std::string asm_fn) { icache.init_mem_asm(asm_fn); }

Classical_part::Classical_part(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , classical_pipeline("Classical_pipeline")
    , meas_reg_file("meas_reg_file")
    , icache("icache") {

    auto logger = get_logger_or_exit("console");

    logger->trace("Start initializing {}...", this->name());

    config();

    // internal vector signal initialization
    MRF2Clp_valid.init(num_qubits);
    MRF2Clp_data.init(num_qubits);

    // ------------------------------------------------------------------------------------
    // Classical pipeline
    // ------------------------------------------------------------------------------------
    classical_pipeline.clock(in_clock);
    classical_pipeline.reset(reset);

    // this signal indicates the signal_initiate process, which initiate relevant signals
    classical_pipeline.init(init);  // port

    // interface to the user
    classical_pipeline.App2Clp_init_pc(App2Clp_init_pc);  // port
    classical_pipeline.Clp2App_done(Clp2App_done);        // port

    // instructions sent from classical pipeline to quantum pipeline
    // assisted by handshaking.
    classical_pipeline.Qp2Clp_ready(Qp2Clp_ready);  // port
    classical_pipeline.Clp2Qp_valid(Clp2Qp_valid);  // port
    classical_pipeline.Clp2Qp_insn(Clp2Qp_insn);    // port
    classical_pipeline.Clp2Qp_Rs(Clp2Qp_Rs);        // port

    // branch related signals to the instruction cache
    classical_pipeline.Clp2Ic_branching(Clp2Ic_branching);  // internal
    classical_pipeline.Clp2Ic_target(Clp2Ic_target);        // internal

    // instructions sent from icache to classical pipeline, assisted by handshaking.
    classical_pipeline.Clp2Ic_ready(Clp2Ic_ready);         // internal
    classical_pipeline.IC2Clp_valid(IC2Clp_valid);         // internal
    classical_pipeline.IC2Clp_branch(IC2Clp_branch_done);  // internal
    classical_pipeline.insn(IC2Clp_insn);                  // internal

    // feedback control related interface
    classical_pipeline.Clp2MRF_meas_issue(Clp2MRF_meas_issue);  // internal
    classical_pipeline.MRF2Clp_ready(MRF2Clp_ready);            // internal
    classical_pipeline.MRF2Clp_valid(MRF2Clp_valid);            // internal
    classical_pipeline.MRF2Clp_data(MRF2Clp_data);              // internal

    // ------------------------------------------------------------------------------------
    // Instruction Cache
    // This module is invisible outside the classical part. so no ports
    // ------------------------------------------------------------------------------------
    icache.clock(in_clock);
    icache.reset(reset);

    // branch & target sent from classical pipeline to icache
    icache.Clp2Sl_branching(Clp2Ic_branching);
    icache.Clp2Sl_target(Clp2Ic_target);

    // instructions sent from icache to classical pipeline, assisted by handshaking.
    icache.Clp2Sl_ready(Clp2Ic_ready);
    icache.Sl2Clp_valid(IC2Clp_valid);
    icache.Sl2Clp_br_done(IC2Clp_branch_done);
    icache.Sl2Clp_insn(IC2Clp_insn);

    // ------------------------------------------------------------------------------------
    // Measurement Register File - Serving Comprehensive Feedback Control
    // ------------------------------------------------------------------------------------
    meas_reg_file.clock(in_clock);
    meas_reg_file.reset(reset);

    // interface to classical pipeline
    meas_reg_file.Clp2MRF_meas_issue(Clp2MRF_meas_issue);  // internal
    meas_reg_file.MRF2Clp_ready(MRF2Clp_ready);            // internal
    meas_reg_file.MRF2Clp_valid(MRF2Clp_valid);            // internal
    meas_reg_file.MRF2Clp_data(MRF2Clp_data);              // internal

    // interface to quantum pipeline
    meas_reg_file.Qp2MRF_meas_issue(Qp2MRF_meas_issue);    // port
    meas_reg_file.Qp2MRF_meas_cancel(Qp2MRF_meas_cancel);  // port

    // interface to the measurement discrimination device
    meas_reg_file.Qm2MRF_meas_result(Qm2MRF_meas_result);  // port

    logger->trace("Finished initializing {}.", this->name());
}

Classical_part::~Classical_part() {}
}  // end of namespace cactus
