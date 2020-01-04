#include "classical_fetch.h"

#include "num_util.h"

namespace cactus {

void Classical_fetch::config(){
    // reserved for future used
};

Classical_fetch::Classical_fetch(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    // instruction fetch stage
    SC_THREAD(signal_update);
    sensitive << reset << App2Clp_init_pc << IC2Clp_branch << IC2Clp_valid << Qp2Clp_ready << insn;

    SC_THREAD(pc_adder);
    sensitive << de_pc << de_insn_valid << de_run;

    SC_THREAD(start_up_logic);
    sensitive << de_clk_en << de_reset_dly << de_reset;

    SC_THREAD(branch_target_adder);
    sensitive << de_pc << de_insn;

    SC_THREAD(branch_latency_control);
    sensitive << de_run << if_br_done << if_br_done_valid << de_br_start << de_insn_valid;

    logger->trace("Finished initializing {}...", this->name());
}

Classical_fetch::~Classical_fetch() {}

void Classical_fetch::signal_update() {
    while (true) {
        wait();

        if_init_pc.write(App2Clp_init_pc.read());
        if_reset.write(reset.read());
        if_br_done.write(IC2Clp_branch.read());
        if_br_done_valid.write(IC2Clp_valid.read());

        de_insn_valid.write(IC2Clp_valid.read());
        de_insn.write(insn.read());

        de_qp_ready = Qp2Clp_ready.read();
    }
}

void Classical_fetch::pc_adder() {
    while (true) {
        wait();

        if (!de_insn_valid.read() || !de_run.read()) {
            if_normal_pc.write(de_pc.read());
            // out_if_normal_pc.write(de_pc.read());
        } else {
            if_normal_pc.write(de_pc.read() + 1);
            // out_if_normal_pc.write(de_pc.read() + 1);
        }
    }
}

void Classical_fetch::start_up_logic() {
    while (true) {
        wait();

        if_reset_dly.write((!de_clk_en.read() & de_reset_dly) | de_reset);
    }
}

void Classical_fetch::branch_target_adder() {
    Qasm_instruction             insn_v;
    sc_int<MEMORY_ADDRESS_WIDTH> br_addr;
    while (true) {
        wait();

        insn_v  = de_insn.read();
        br_addr = insn_v.get_br_addr();

        if (de_reset_dly) {
            if_target_pc.write(de_init_pc);
            // out_if_target_pc.write(de_init_pc);
        } else {
            if_target_pc.write(de_pc.read() + br_addr);
            // out_if_target_pc.write(de_pc.read() + br_addr);
        }
    }
}

// this signal goes low during branches and after a stop instruction
void Classical_fetch::branch_latency_control() {
    while (true) {
        wait();

        if_run.write(de_run.read());

        if (de_insn_valid.read()) {

            // zero latency
            if (de_br_start) if_run.write(0);

            if (if_br_done_valid.read() && if_br_done.read()) if_run.write(1);
        }
    }
}

}  // namespace cactus
