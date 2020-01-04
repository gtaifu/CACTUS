#include "q_tech_ind.h"

namespace cactus {

void Q_tech_ind::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits       = global_config.num_qubits;
    m_vliw_width       = global_config.vliw_width;
    m_instruction_type = global_config.instruction_type;
}

Q_tech_ind::Q_tech_ind(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , op_combiner("Operation_combiner")
    , vec_vliw_pipelane("vliw_pipelane")
    , meas_issue_gen("meas_issue_gen") {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    // instance sub modules
    if (m_instruction_type == Instruction_type::BIN) {
        q_decoder = new Q_decoder_bin("q_decoder_bin");
    } else {
        q_decoder = new Q_decoder_asm("q_decoder_asm");
    }

    // initial IO vector signals
    vec_vliw_pipelane.init(m_vliw_width,
                           [&](const char* nm, size_t i) { return new Vliw_pipelane(nm); });

    vec_vliw_in_pipe_sig.init(m_vliw_width);
    vec_vliw_out_pipe_sig.init(m_vliw_width);

    // ------------------------------------------------------------------------------------------
    // quantum binary instruction decoder
    // ------------------------------------------------------------------------------------------
    // input
    q_decoder->in_clock(in_clock);
    q_decoder->reset(reset);
    q_decoder->in_bundle(in_bundle);
    q_decoder->in_valid_bundle(in_valid_bundle);
    q_decoder->in_rs_wait_time(in_rs_wait_time);

    // output
    q_decoder->out_q_pipe_interface(q_pipe_interface_sig);

    // ------------------------------------------------------------------------------------------
    // operation combiner
    // ------------------------------------------------------------------------------------------
    // input
    op_combiner.in_clock(in_clock);
    op_combiner.reset(reset);

    // interface to vliw pipelane
    for (size_t i = 0; i < m_vliw_width; ++i) {
        op_combiner.vec_in_q_pipe_interface[i](vec_vliw_out_pipe_sig[i]);
    }

    // output
    op_combiner.out_q_pipe_interface(out_q_pipe_interface);

    // ------------------------------------------------------------------------------------------
    // vliw pipelane
    // ------------------------------------------------------------------------------------------
    for (size_t i = 0; i < m_vliw_width; ++i) {
        // input
        vec_vliw_pipelane[i].in_clock(in_clock);
        vec_vliw_pipelane[i].reset(reset);
        vec_vliw_pipelane[i].in_q_pipe_interface(vec_vliw_in_pipe_sig[i]);
        // output
        vec_vliw_pipelane[i].out_q_pipe_interface(vec_vliw_out_pipe_sig[i]);
    }

    // ------------------------------------------------------------------------------------------
    // meas_issue_gen
    // ------------------------------------------------------------------------------------------
    // input
    meas_issue_gen.in_clock(in_clock);
    meas_issue_gen.reset(reset);
    meas_issue_gen.in_q_pipe_interface(out_q_pipe_interface);

    // output
    meas_issue_gen.out_Qp2MRF_meas_issue(out_Qp2MRF_meas_issue);

    // methods
    SC_CTHREAD(do_output, in_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Q_tech_ind::~Q_tech_ind() {
    if (q_decoder) {
        delete q_decoder;
        q_decoder = nullptr;
    }
}

void Q_tech_ind::do_output() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface tmp_in_q_pipe_interface;
    Q_pipe_interface tmp_out_q_pipe_interface;

    while (true) {
        wait();

        tmp_in_q_pipe_interface.reset();  // clear everything at the begin of every cycle

        if (!reset.read()) {

            tmp_in_q_pipe_interface = q_pipe_interface_sig.read();

            if (tmp_in_q_pipe_interface.if_content.valid_wait) {  // generate new timing point

                if ((tgt_timing_type == TIMING_POINT) &&
                    (tmp_in_q_pipe_interface.timing.type == WAIT_TIME)) {

                    current_timing.type      = TIMING_POINT;
                    current_timing.wait_time = tmp_in_q_pipe_interface.timing.wait_time;
                    current_timing.label++;  // update timing label

                    logger->debug("{}: generate new timing point,wait time:0x{:x},label:0x{:x}",
                                  this->name(), current_timing.wait_time, current_timing.label);
                } else {
                    logger->error(
                      "{}: Unsupported timing type convertion,source_type:{},"
                      "target_type:{}",
                      this->name(), tmp_in_q_pipe_interface.timing.type, tgt_timing_type);
                    exit(EXIT_FAILURE);
                }
            } else {  // do not generate new timing point,still use the previous timing
            }

            // distribute operations to each vliw pipelane
            tmp_out_q_pipe_interface        = tmp_in_q_pipe_interface;
            tmp_out_q_pipe_interface.timing = current_timing;

            for (size_t i = 0; i < tmp_in_q_pipe_interface.ops.size(); ++i) {
                tmp_out_q_pipe_interface.ops.clear();  // clear operations

                tmp_out_q_pipe_interface.ops.push_back(
                  tmp_in_q_pipe_interface.ops[i]);  // push i-th operation

                vec_vliw_in_pipe_sig[i].write(tmp_out_q_pipe_interface);
            }

        } else {
            current_timing.type      = TIMING_POINT;
            current_timing.wait_time = 0;
            current_timing.label     = 0;  // reset timing label

            for (size_t i = 0; i < m_vliw_width; ++i) {
                vec_vliw_in_pipe_sig[i].write(tmp_in_q_pipe_interface);
            }
        }
    }
}

}  // end of namespace cactus