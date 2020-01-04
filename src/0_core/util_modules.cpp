#include "util_modules.h"

namespace cactus {
// The mux is combinatorial module.
void Operation_mux::do_work() {
    Operation no_operation;
    Op_sel    input_sel;

    while (true) {

        wait();

        input_sel = in_sel.read();
        if (in_valid.read()) {
            switch (input_sel.val) {
                case 0b00:
                    no_operation.type = 0;
                    out_op.write(no_operation);
                    // out_op.write(Operation(OP_TYPE_DEVICE::NOP));
                    break;

                case 0b10:
                    out_op.write(in_op_right);
                    break;

                case 0b01:
                    out_op.write(in_op_left);
                    break;

                case 0b11:
                    out_op.write(in_op_left);
                    break;

                default:
                    assert(false);
            }
        } else {
            no_operation.type = 0;
            out_op.write(no_operation);
            // out_op.write(Operation(OP_TYPE_DEVICE::NOP));
        }
    }
}

void Micro_operation_or::do_work() {
    while (true) {
        wait();

        Micro_operation tmp_u_op(0, 0, 0, 0, 0);

        for (size_t i = 0; i < VLIW_WIDTH; ++i) {
            tmp_u_op = tmp_u_op | in_u_ops[i].read();
        }

        out_u_op.write(tmp_u_op);
    }
}

}  // namespace cactus
