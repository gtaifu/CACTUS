#ifndef _ICACHE_SLICE_H_
#define _ICACHE_SLICE_H_

#include <systemc>

#include "global_json.h"
#include "qasm_instruction.h"

namespace cactus {
using sc_core::sc_fifo;
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;

SC_MODULE(Icache_slice) {
    sc_in<bool>                          clock;
    sc_in<bool>                          reset;
    sc_in<bool>                          Clp2Sl_ready;
    sc_in<bool>                          Clp2Sl_branching;
    sc_in<sc_uint<MEMORY_ADDRESS_WIDTH>> Clp2Sl_target;

    sc_in<bool>             IC2Sl_valid;
    sc_in<bool>             IC2Sl_br_done;
    sc_in<Qasm_instruction> IC2Sl_insn;

    sc_out<bool>                          Sl2Ic_ready;
    sc_out<bool>                          Sl2Ic_branching;
    sc_out<sc_uint<MEMORY_ADDRESS_WIDTH>> Sl2Ic_target;

    sc_out<bool>             Sl2Clp_valid;
    sc_out<bool>             Sl2Clp_br_done;
    sc_out<Qasm_instruction> Sl2Clp_insn;

    void command_slice_gen();
    void no_command_slice();

    void ready_slice_gen();
    void drive_slice_output();
    void no_ready_slice();
    void drive_ready();

    void data_slice_gen();
    void drive_ready_output();
    void drive_data_output();
    void no_data_slice();

  protected:
    sc_signal<bool>             Rsl2Dsl_br_done;
    sc_signal<bool>             Rsl2Dsl_valid;
    sc_signal<Qasm_instruction> Rsl2Dsl_insn;
    sc_signal<bool>             Dsl2Rsl_ready;

    sc_signal<Qasm_instruction> Rsl_buf_insn;
    sc_signal<bool>             Rsl_buf_br_done;
    sc_signal<bool> Rsl_buf_invalid;  // signal that indicates whether the buffer is full

    sc_signal<Qasm_instruction> Dsl_buf_insn;
    sc_signal<bool>             Dsl_buf_br_done;
    sc_signal<bool>             Dsl_buf_valid;

  public:
    SC_CTOR(Icache_slice) {
        auto logger = get_logger_or_exit("cache_logger");
        logger->trace("Start initializing {}...", this->name());

        SC_CTHREAD(command_slice_gen, clock.pos());
        SC_THREAD(no_command_slice);
        sensitive << Clp2Sl_branching << Clp2Sl_target;

        SC_CTHREAD(ready_slice_gen, clock.pos());
        SC_THREAD(drive_slice_output);
        sensitive << Rsl_buf_br_done << Rsl_buf_insn << Rsl_buf_invalid << IC2Sl_br_done
                  << IC2Sl_insn << IC2Sl_valid;
        SC_THREAD(no_ready_slice);
        sensitive << IC2Sl_br_done << IC2Sl_insn << IC2Sl_valid << Dsl2Rsl_ready;
        SC_THREAD(drive_ready);
        sensitive << Rsl_buf_invalid;

        SC_CTHREAD(data_slice_gen, clock.pos());
        SC_THREAD(no_data_slice);
        sensitive << Rsl2Dsl_br_done << Rsl2Dsl_insn << Rsl2Dsl_valid;
        SC_THREAD(drive_ready_output);
        sensitive << Dsl_buf_valid << Clp2Sl_ready;
        SC_THREAD(drive_data_output);
        sensitive << Dsl_buf_br_done << Dsl_buf_insn << Dsl_buf_valid;

        logger->trace("Finished initializing {}...", this->name());
    }
};
}  // namespace cactus

#endif
