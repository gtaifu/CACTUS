#ifndef _ICACHE_H_
#define _ICACHE_H_

#include <systemc>

#include "global_json.h"
#include "icache_rtl.h"
#include "icache_slice.h"
#include "logger_wrapper.h"

namespace cactus {

SC_MODULE(ICache) {
    sc_in<bool> clock;
    sc_in<bool> reset;

    sc_in<bool>                          Clp2Sl_branching;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Sl_target;

    sc_in<bool>              Clp2Sl_ready;
    sc_out<bool>             Sl2Clp_valid;
    sc_out<bool>             Sl2Clp_br_done;
    sc_out<Qasm_instruction> Sl2Clp_insn;

  public:
    void init_mem_asm(std::string asm_fn);

  protected:  // internal signals
    // branch & target from Slice to Cache
    sc_signal<bool>                          Sl2Ic_branching;
    sc_signal<sc_uint<MEMORY_ADDRESS_WIDTH>> Sl2Ic_target;

    // ready and valid signals for branch and insn.
    sc_signal<bool>             Sl2Ic_ready;
    sc_signal<bool>             IC2Sl_valid;
    sc_signal<bool>             IC2Sl_br_done;
    sc_signal<Qasm_instruction> IC2Sl_insn;

  protected:  // internal modules
    Icache_slice icache_slice;
    Icache_rtl   icache_rtl;

  public:
    ICache(const sc_core::sc_module_name& n);

};  // end of Icache
}  // namespace cactus
#endif  // _ICACHE_H_
