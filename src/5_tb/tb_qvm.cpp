#include "tb_qvm.h"

namespace cactus {

void QVM_TB::config() {

    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;
};

QVM_TB::QVM_TB(const sc_core::sc_module_name& n, unsigned int num_sim_cycles_)
    : Quma_tb_base(n, num_sim_cycles_)
    , qvm("qvm") {

    auto logger = spdlog::get("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    m_num_sim_cycles = num_sim_cycles_;

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

}  // namespace cactus
