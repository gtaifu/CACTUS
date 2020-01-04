#include "icache_top.h"

namespace cactus {

ICache::ICache(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , icache_slice("icache_slice")
    , icache_rtl("icache_rtl") {

    auto logger = get_logger_or_exit("cache_logger");
    logger->trace("Start initializing {}...", this->name());

    icache_slice.clock(clock);
    icache_slice.reset(reset);

    icache_slice.Clp2Sl_branching(Clp2Sl_branching);
    icache_slice.Clp2Sl_target(Clp2Sl_target);

    icache_slice.Sl2Ic_branching(Sl2Ic_branching);
    icache_slice.Sl2Ic_target(Sl2Ic_target);

    icache_slice.Sl2Clp_br_done(Sl2Clp_br_done);
    icache_slice.Sl2Clp_insn(Sl2Clp_insn);

    icache_slice.Clp2Sl_ready(Clp2Sl_ready);
    icache_slice.Sl2Clp_valid(Sl2Clp_valid);

    icache_slice.Sl2Ic_ready(Sl2Ic_ready);
    icache_slice.IC2Sl_valid(IC2Sl_valid);
    icache_slice.IC2Sl_br_done(IC2Sl_br_done);
    icache_slice.IC2Sl_insn(IC2Sl_insn);

    icache_rtl.clock(clock);
    icache_rtl.reset(reset);

    icache_rtl.Clp2Ic_branching(Sl2Ic_branching);
    icache_rtl.Clp2Ic_target(Sl2Ic_target);

    icache_rtl.Clp2Ic_ready(Sl2Ic_ready);

    icache_rtl.IC2Clp_valid(IC2Sl_valid);
    icache_rtl.IC2Clp_br_done(IC2Sl_br_done);
    icache_rtl.IC2Clp_insn(IC2Sl_insn);

    logger->trace("Finished initializing {}...", this->name());
}

void ICache::init_mem_asm(std::string asm_fn) { icache_rtl.init_mem_asm(asm_fn); }

}  // end of namespace cactus
