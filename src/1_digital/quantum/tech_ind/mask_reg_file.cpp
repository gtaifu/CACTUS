#include "mask_reg_file.h"

#include "num_util.h"

namespace cactus {
void Mask_register_file::config() {
    Global_config& global_config = Global_config::get_instance();

    m_vliw_width = global_config.vliw_width;
}

Mask_register_file::Mask_register_file(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    SC_CTHREAD(do_write, in_clock.pos());
    SC_CTHREAD(do_read, in_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

void Mask_register_file::do_write() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface q_pipe_interface;
    Q_tgt_addr       addr_to_set;

    while (true) {
        wait();

        q_pipe_interface.reset();  // clear everything at the begin of every cycle

        if (!reset.read()) {

            q_pipe_interface = in_q_pipe_interface.read();
        } else {
            q_mask_reg.reset();
        }

        if (!q_pipe_interface.if_content.valid_set_addr) continue;

        // update register
        for (size_t i = 0; i < q_pipe_interface.addrs_to_set.size(); ++i) {

            addr_to_set = q_pipe_interface.addrs_to_set[i];
            if (addr_to_set.type.c_type == INDIRECT_REG_NUM) {  // register addressing

                if (addr_to_set.type.q_num_type ==
                    SINGLE) {  // update register which used for single-qubit operation
                    q_mask_reg.set_reg_mask(addr_to_set.sq_op_addr.mask,
                                            addr_to_set.indirect_addr_reg_num,
                                            addr_to_set.type.q_num_type);

                    logger->debug("{}: update register,type:single,reg_num:{},mask:0x{:x}",
                                  this->name(), addr_to_set.indirect_addr_reg_num.get_value(),
                                  addr_to_set.sq_op_addr.mask.get_value());

                } else {  // update register which used for multi-qubit operation
                    q_mask_reg.set_reg_mask(addr_to_set.mq_op_addr.mask,
                                            addr_to_set.indirect_addr_reg_num,
                                            addr_to_set.type.q_num_type);

                    logger->debug("{}: update register,type:multiple,reg_num:{}, mask:0x{:x}",
                                  this->name(), addr_to_set.indirect_addr_reg_num.get_value(),
                                  addr_to_set.mq_op_addr.mask.get_value());
                }
            } else if (addr_to_set.type.c_type == INDIRECT_REG_CONTENT) {
                if (addr_to_set.type.q_num_type == SINGLE) {
                    // update register which used for single-qubit operation
                    q_mask_reg.set_s_reg_content(addr_to_set.sq_op_addr.qubit_indices,
                                                 addr_to_set.indirect_addr_reg_num);
                } else {
                    // update register which used for multi-qubit operation
                    q_mask_reg.set_m_reg_content(addr_to_set.mq_op_addr.qubit_tuples,
                                                 addr_to_set.indirect_addr_reg_num);
                }
            } else {
                // other addressing mode do not need write register
            }
        }
    }
}

void Mask_register_file::do_read() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface q_pipe_interface;
    Fledged_qop      qop;

    while (true) {
        wait();

        q_pipe_interface.reset();  // clear everything at the begin of every cycle

        if (reset.read()) {  // when received reset sig
            out_q_pipe_interface.write(q_pipe_interface);
            continue;
        }

        q_pipe_interface = in_q_pipe_interface.read();

        if (q_pipe_interface.if_content.valid_qop) {

            // read register
            qop = q_pipe_interface.ops[0];
            if (qop.addr.type.c_type == INDIRECT_REG_NUM) {
                // single-qubit operation
                if (qop.addr.type.q_num_type == SINGLE) {
                    qop.addr.sq_op_addr.mask = q_mask_reg.get_reg_mask(
                      qop.addr.indirect_addr_reg_num, qop.addr.type.q_num_type);

                    logger->debug("{}: read register,type:single,reg_num:{},mask:0x{:x}",
                                  this->name(), qop.addr.indirect_addr_reg_num.get_value(),
                                  qop.addr.sq_op_addr.mask.get_value());
                } else {
                    // multi-qubit operation
                    qop.addr.mq_op_addr.mask = q_mask_reg.get_reg_mask(
                      qop.addr.indirect_addr_reg_num, qop.addr.type.q_num_type);

                    logger->debug("{}: read register,type:multiple,reg_num:{},mask:0x{:x}",
                                  this->name(), qop.addr.indirect_addr_reg_num.get_value(),
                                  qop.addr.mq_op_addr.mask.get_value());
                }
            } else if (qop.addr.type.c_type == INDIRECT_REG_CONTENT) {
                if (qop.addr.type.q_num_type == SINGLE) {
                    // single-qubit operation
                    qop.addr.sq_op_addr.qubit_indices =
                      q_mask_reg.get_s_reg_content(qop.addr.indirect_addr_reg_num);

                } else {
                    // multi-qubit operation
                    qop.addr.mq_op_addr.qubit_tuples =
                      q_mask_reg.get_m_reg_content(qop.addr.indirect_addr_reg_num);
                }

            } else {
                // other addressing mode donot need read register
            }

            q_pipe_interface.ops.clear();
            q_pipe_interface.ops.push_back(qop);
        } else {
            // other addressing mode do not need read register
        }

        q_pipe_interface.if_content.valid_set_addr = false;  // make it false for subsequent modules
        out_q_pipe_interface.write(q_pipe_interface);
    }
}

void Mask_register_file::add_telf_header() {}

void Mask_register_file::add_telf_line() {}

}  // end of namespace cactus
