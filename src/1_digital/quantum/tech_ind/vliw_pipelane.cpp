#include "vliw_pipelane.h"

namespace cactus {

void Vliw_pipelane::config() {}

Vliw_pipelane::Vliw_pipelane(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , addr_mask_decoder("addr_decoder")
    , mask_reg_file("mask_register_file")
    , op_decoder("op_deocder") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    // ------------------------------------------------------------------------------------------
    // mask register file
    // ------------------------------------------------------------------------------------------
    // input
    mask_reg_file.in_clock(in_clock);
    mask_reg_file.reset(reset);
    mask_reg_file.in_q_pipe_interface(in_q_pipe_interface);

    // interface to addr mask decoder
    mask_reg_file.out_q_pipe_interface(register_addressing_pipe_sig);

    // ------------------------------------------------------------------------------------------
    // addr mask decoder
    // ------------------------------------------------------------------------------------------
    // input
    addr_mask_decoder.in_clock(in_clock);
    addr_mask_decoder.reset(reset);

    // interface to mask register file
    addr_mask_decoder.in_q_pipe_interface(register_addressing_pipe_sig);

    // output
    addr_mask_decoder.out_q_pipe_interface(mask_decode_pipe_sig);

    // ------------------------------------------------------------------------------------------
    // operation decoder
    // ------------------------------------------------------------------------------------------
    // input
    op_decoder.in_q_pipe_interface(mask_decode_pipe_sig);

    // output
    op_decoder.out_q_pipe_interface(out_q_pipe_interface);

    logger->trace("Finished initializing {}...", this->name());
}

}  // namespace cactus
