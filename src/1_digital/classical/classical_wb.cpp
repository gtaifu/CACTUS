#include "classical_wb.h"

#include "num_util.h"

namespace cactus {

void Classical_wb::config() {

    Global_config& global_config = Global_config::get_instance();

    m_output_dir = global_config.output_dir;
    is_telf_on   = true;
    telf_fn      = sep_telf_fn(global_config.output_dir, this->name(), "classical_wb");
};

Classical_wb::Classical_wb(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();
    open_telf_file();

    reg_file.init(REG_FILE_NUM);

    // writeback stage
    SC_CTHREAD(mem2wb_ff, clock.pos());
    SC_THREAD(write_reg_file);
    sensitive << wb_wr_rd_en << wb_rd_addr << wb_rd_value << init;

    SC_CTHREAD(clock_counter, clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Classical_wb::~Classical_wb() { close_telf_file(); }

void Classical_wb::mem2wb_ff() {
    while (true) {
        wait();

        wb_insn.write(mem_insn.read());
        wb_wr_rd_en.write(mem_wr_rd_en.read());
        wb_rd_addr.write(mem_rd_addr.read());
        wb_run.write(mem_run.read());
        if (mem_ex2reg.read()) {
            wb_rd_value.write(mem_ex_rd_value.read());
        } else {
            // especially for load instr
            wb_rd_value.write(mem_rd_value.read());
        }
    }
}

void Classical_wb::write_reg_file() {

    while (true) {
        wait();

        // initial register file
        if (init.read()) {
            for (size_t i = 0; i < REG_FILE_WIDTH; ++i) {
                reg_file[i] = 0;
            }
        }

        // write back register
        if (wb_run) {
            if (wb_wr_rd_en) {
                reg_file[static_cast<size_t>(wb_rd_addr.read())] = wb_rd_value.read();

                // Write the text output
                if (is_telf_on) {
                    telf_os << std::setfill(' ') << std::setw(15)
                            << sc_core::sc_time_stamp().to_string();
                    telf_os << ",    " << std::setfill(' ') << std::setw(11) << m_num_cycles;
                    telf_os << ",    " << std::setfill(' ') << std::setw(13) << wb_rd_addr.read();
                    telf_os << ",    " << std::setfill(' ') << std::setw(14) << wb_rd_value.read();
                    telf_os << std::endl;
                }
            }
        }
    }
}

void Classical_wb::clock_counter() {
    while (true) {
        wait();
        m_num_cycles++;
    }
}

void Classical_wb::add_telf_header() {

    // Specify the signal type
    telf_os << "#RegValue" << std::endl;

    telf_os << "       sim time,    Clock cycle,    R register number,    register value"
            << std::endl;
}

}  // namespace cactus
