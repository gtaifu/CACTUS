#include "icache_slice.h"

namespace cactus {

void Icache_slice::command_slice_gen() {
    while (true) {
        wait();

        if (G_CMD_SLICE == 1) {
            Sl2Ic_branching.write(Clp2Sl_branching.read());
            Sl2Ic_target.write(Clp2Sl_target.read());

            if (reset.read()) {
                Sl2Ic_branching.write(0);
            }
        }
    }
}

void Icache_slice::no_command_slice() {
    while (true) {
        wait();

        if (G_CMD_SLICE == 0) {
            Sl2Ic_branching.write(Clp2Sl_branching.read());
            Sl2Ic_target.write(Clp2Sl_target.read());
        }
    }
}

void Icache_slice::ready_slice_gen() {
    while (true) {
        wait();

        if (G_READY_SLICE == 1) {
            if (Rsl_buf_invalid.read() && IC2Sl_valid.read() && !Dsl2Rsl_ready.read()) {
                // load the buffer
                Rsl_buf_insn    = IC2Sl_insn.read();
                Rsl_buf_br_done = IC2Sl_br_done.read();
                Rsl_buf_invalid.write(0);
            } else if (!Rsl_buf_invalid.read() && Dsl2Rsl_ready.read()) {
                Rsl_buf_invalid.write(1);
            }

            if (reset.read()) Rsl_buf_invalid.write(1);
        }
    }
}

void Icache_slice::drive_slice_output() {
    while (true) {
        wait();

        if (G_READY_SLICE == 1) {
            if (!Rsl_buf_invalid.read()) {
                // supply the contents of the buffer
                Rsl2Dsl_br_done = Rsl_buf_br_done.read();
                Rsl2Dsl_insn    = Rsl_buf_insn.read();
                Rsl2Dsl_valid.write(1);
            } else {
                // drive the output directly
                Rsl2Dsl_br_done = IC2Sl_br_done.read();
                Rsl2Dsl_insn    = IC2Sl_insn.read();
                Rsl2Dsl_valid   = IC2Sl_valid.read();
            }
        }
    }
}

void Icache_slice::no_ready_slice() {
    while (true) {
        wait();

        if (G_READY_SLICE == 0) {
            Rsl2Dsl_br_done = IC2Sl_br_done.read();
            Rsl2Dsl_insn    = IC2Sl_insn.read();
            Rsl2Dsl_valid   = IC2Sl_valid.read();
            Sl2Ic_ready     = Dsl2Rsl_ready.read();
        }
    }
}

void Icache_slice::drive_ready() {
    while (true) {
        wait();

        if (G_READY_SLICE == 1) {
            // Tie our ready output to the buffer state register
            Sl2Ic_ready = Rsl_buf_invalid.read();
        }
    }
}

void Icache_slice::data_slice_gen() {

    while (true) {
        wait();

        if (G_DATA_SLICE == 1) {
            if (!Dsl_buf_valid.read() && Rsl2Dsl_valid.read()) {
                Dsl_buf_br_done = Rsl2Dsl_br_done.read();
                Dsl_buf_insn    = Rsl2Dsl_insn.read();
                Dsl_buf_valid.write(1);
            } else if (Dsl_buf_valid.read() && Clp2Sl_ready.read()) {
                Dsl_buf_br_done = Rsl2Dsl_br_done.read();
                Dsl_buf_insn    = Rsl2Dsl_insn.read();
                Dsl_buf_valid   = Rsl2Dsl_valid.read();
            }

            if (reset.read()) {
                Dsl_buf_valid.write(0);
            }
        }
    }
}

void Icache_slice::drive_ready_output() {
    while (true) {
        wait();

        if (G_DATA_SLICE == 1) {
            if (!Dsl_buf_valid.read() || Clp2Sl_ready.read()) {
                Dsl2Rsl_ready.write(1);
            } else {
                Dsl2Rsl_ready.write(0);
            }
        }
    }
}

void Icache_slice::drive_data_output() {
    while (true) {
        wait();

        if (G_DATA_SLICE == 1) {
            Sl2Clp_br_done = Dsl_buf_br_done.read();
            Sl2Clp_insn    = Dsl_buf_insn.read();
            Sl2Clp_valid   = Dsl_buf_valid.read();
        }
    }
}

void Icache_slice::no_data_slice() {
    while (true) {
        wait();

        if (G_DATA_SLICE == 0) {
            Sl2Clp_br_done = Rsl2Dsl_br_done.read();
            Sl2Clp_insn    = Rsl2Dsl_insn.read();
            Sl2Clp_valid   = Rsl2Dsl_valid.read();
            Dsl2Rsl_ready  = Clp2Sl_ready.read();
        }
    }
}

}  // namespace cactus
