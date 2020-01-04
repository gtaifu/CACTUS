#include "classical_mem.h"

#include "num_util.h"

namespace cactus {

void Classical_mem::config() {
    auto logger = get_logger_or_exit("console");

    Global_config& global_config = Global_config::get_instance();

    m_output_dir    = global_config.output_dir;
    m_data_mem      = global_config.data_memory;
    m_data_mem_size = m_data_mem->get_mem_size();

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "classical_mem");
};

Classical_mem::Classical_mem(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();
    open_telf_file();

    flags_cmp.init(16);
    flags_cmp_dly.init(16);

    // mem stage
    SC_CTHREAD(ex2mem_ff, clock.pos());
    // read from memory
    SC_CTHREAD(read_mem, clock.pos());
    // write to memory
    SC_CTHREAD(write_mem, clock.pos());
    // sign extend
    SC_THREAD(sign_extend);
    sensitive << mem_sext << mem_addr_sel << mem_out_data;

    SC_CTHREAD(clock_counter, clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

Classical_mem::~Classical_mem() { close_telf_file(); }

void Classical_mem::ex2mem_ff() {
    while (true) {
        wait();

        mem_insn.write(ex_insn.read());
        mem_ex_rd_value.write(ex_rd_value.read());
        mem_wr_rd_en.write(ex_wr_rd_en.read());
        mem_rd_addr.write(ex_rd_addr.read());
        mem_run.write(ex_run.read());
        mem_ex2reg.write(ex_ex2reg.read());
        mem_addr_sel.write(ex_mem_addr_sel.read());
        mem_sext.write(ex_mem_sext.read());

        // cond_result.write(ex_cond_result.read());
        for (size_t i = 0; i < 16; ++i) {
            flags_cmp_dly[i].write(flags_cmp[i].read());
        }
    }
}

void Classical_mem::read_mem() {
    auto logger = get_logger_or_exit("console");

    unsigned int read_data;
    bool         need_data_next = false;
    unsigned int read_data_next;
    unsigned int read_addr;
    unsigned int byte_sel;
    std::string  addr_sel_for_log;
    while (true) {
        wait();

        if (ex_run.read() && ex_mem_strobe.read() && ex_mem_rw.read()) {

            read_addr = ex_rd_value.read().to_uint();
            if (read_addr >= m_data_mem_size) {
                logger->error(
                  "{}: Memory read address '0x{:08x}' is out of data memory size '0x{:08x}'.  "
                  "Simulation aborts!",
                  this->name(), read_addr, m_data_mem_size);
                exit(EXIT_FAILURE);
            }

            read_data      = m_data_mem->read_mem(read_addr >> 2);
            byte_sel       = read_addr & 0x3;
            need_data_next = false;

            // check whether read address is out of memory when read a word
            if (ex_mem_addr_sel.read() == ADDR_WORD) {
                if (read_addr > m_data_mem_size - 4) {
                    logger->error(
                      "{}: Memory read address '0x{:08x}' is out of data memory size '0x{:08x}'.  "
                      "Simulation aborts!",
                      this->name(), read_addr, m_data_mem_size);
                    exit(EXIT_FAILURE);
                }
                if (byte_sel != 0) {
                    need_data_next = true;
                }
            }

            // check whether read address is out of memory when read a half word
            if (ex_mem_addr_sel.read() == ADDR_HALF_WORD) {
                if (read_addr > m_data_mem_size - 2) {
                    logger->error(
                      "{}: Memory read address '0x{:08x}' is out of data_memory size '0x{:08x}'.  "
                      "Simulation aborts!",
                      this->name(), read_addr, m_data_mem_size);
                    exit(EXIT_FAILURE);
                }
                if (byte_sel == 3) {
                    need_data_next = true;
                }
            }

            if (need_data_next) {
                read_data_next = m_data_mem->read_mem((read_addr >> 2) + 1);
            }

            switch (ex_mem_addr_sel.read()) {
                case ADDR_BYTE:  // byte access
                    switch (byte_sel) {
                        case 0:
                            read_data = read_data & 0x000000ff;
                            break;
                        case 1:
                            read_data = (read_data & 0x0000ff00) >> 8;
                            break;
                        case 2:
                            read_data = (read_data & 0x00ff0000) >> 16;
                            break;
                        case 3:
                            read_data = (read_data & 0xff000000) >> 24;
                            break;
                        default:
                            break;
                    }
                    addr_sel_for_log = "byte";
                    break;
                case ADDR_HALF_WORD:  // half word access, not necessary aligned to 2
                    switch (byte_sel) {
                        case 0:
                            read_data = read_data & 0x0000ffff;
                            break;
                        case 1:
                            read_data = (read_data & 0x00ffff00) >> 8;
                            break;
                        case 2:
                            read_data = (read_data & 0xffff0000) >> 16;
                            break;
                        case 3:
                            read_data = ((read_data_next & 0x000000ff) << 8) |
                                        ((read_data & 0xff000000) >> 24);
                            break;
                        default:
                            break;
                    }
                    addr_sel_for_log = "half";
                    break;
                case ADDR_WORD:  // word access, not necessary aligned to 4
                    switch (byte_sel) {
                        case 0:
                            // data has been read before switch statement
                            break;
                        case 1:
                            read_data = (read_data >> 8) | ((read_data_next & 0x000000ff) << 24);
                            break;
                        case 2:
                            read_data = (read_data >> 16) | ((read_data_next & 0x0000ffff) << 16);
                            break;
                        case 3:
                            read_data = (read_data >> 24) | ((read_data_next & 0x00ffffff) << 8);
                            break;
                        default:
                            break;
                    }
                    addr_sel_for_log = "word";
                    break;
                    /* aligned to 2
                    case ADDR_HALF_WORD:  // half word access
                        if ((ex_rd_value.read() & 0x1) != 0) {
                            // not 2 aligned
                            logger->error(
                              "{}: memory read address (0x{:8x}) is not aligned to 2 when
                    write half " "word. Simulation aborts!", this->name(),
                    ex_rd_value.read()); exit(EXIT_FAILURE);
                        }

                        // upper 16 bit
                        if ((ex_rd_value.read() & 0x2) != 0) {
                            read_data = (read_data & 0xffff0000) >> 16;
                        } else {
                            // lower 16 bit
                            read_data = read_data & 0x0000ffff;
                        }
                        addr_sel_for_log = "half";
                        break;
                    */
                    /* aligned to 4
                     case ADDR_WORD:  // word access
                         if ((ex_rd_value.read() & 0x3) != 0) {
                             // not 4 aligned
                             logger->error(
                               "{}: memory write address (0x{:8x}) is not aligned to 4 when
                     write a " "word. Simulation aborts!", this->name(),
                     ex_rd_value.read()); exit(EXIT_FAILURE);
                         }
                         addr_sel_for_log = "word";
                         // data has been read before switch statement
                         break;
                     */
                default:
                    logger->error(
                      "{}: Unrecognized memory access type. Support types: Byte, half word, word. "
                      "Simulation aborts!",
                      this->name());
                    exit(EXIT_FAILURE);
                    break;
            }
            mem_out_data.write(read_data);

            // for logging
            if (is_telf_on) {
                telf_os << std::setfill(' ') << std::setw(15)
                        << sc_core::sc_time_stamp().to_string();
                telf_os << " " << std::setfill(' ') << std::setw(11) << std::dec << m_num_cycles;
                telf_os << " " << std::setfill(' ') << std::setw(3) << "r";
                telf_os << " " << std::setfill(' ') << std::setw(10) << addr_sel_for_log;
                telf_os << " " << std::setfill('0') << std::setw(8) << std::hex << read_addr;
                telf_os << " " << std::setfill('0') << std::setw(8) << std::hex << read_data;
                telf_os << std::endl;
            }
        }
    }
}

void Classical_mem::write_mem() {
    auto logger = get_logger_or_exit("console");

    unsigned int m_data;
    bool         need_data_next = false;
    unsigned int m_data_next;
    unsigned int write_addr;

    std::string  addr_sel_for_log;  // for logging
    unsigned int data_for_log;      // for logging

    while (true) {
        wait();

        // store instr
        if (ex_run.read() && ex_mem_strobe.read() && !ex_mem_rw.read()) {

            write_addr = ex_rd_value.read().to_uint();

            if (write_addr >= m_data_mem_size) {
                logger->error(
                  "{}: Memory write address '0x{:08x}' is out of data memory size '0x{:08x}'.  "
                  "Simulation aborts!",
                  this->name(), write_addr, m_data_mem_size);
                exit(EXIT_FAILURE);
            }

            m_data                = m_data_mem->read_mem(write_addr >> 2);
            unsigned int byte_sel = ex_rd_value.read() & 0x3;
            need_data_next        = false;

            // check whether read address is out of memory when read a word
            if (ex_mem_addr_sel.read() == ADDR_WORD) {
                if (write_addr > m_data_mem_size - 4) {
                    logger->error(
                      "{}: Memory write address '0x{:08x}' is out of data_memory size '0x{:08x}'.  "
                      "Simulation aborts!",
                      this->name(), write_addr, m_data_mem_size);
                    exit(EXIT_FAILURE);
                }
                if (byte_sel != 0) {
                    need_data_next = true;
                }
            }

            // check whether read address is out of memory when read a half word
            if (ex_mem_addr_sel.read() == ADDR_HALF_WORD) {
                if (write_addr > m_data_mem_size - 2) {
                    logger->error(
                      "{}: Memory write address '0x{:08x}' is out of data memory size '0x{:08x}'.  "
                      "Simulation aborts!",
                      this->name(), write_addr, m_data_mem_size);
                    exit(EXIT_FAILURE);
                }
                if (byte_sel == 3) {
                    need_data_next = true;
                }
            }

            if (need_data_next) {
                m_data_next = m_data_mem->read_mem((write_addr >> 2) + 1);
            }

            switch (ex_mem_addr_sel.read()) {
                case ADDR_BYTE:  // byte access
                    switch (byte_sel) {
                        case 0:
                            m_data =
                              (m_data & 0xffffff00) | ex_mem_data.read().range(7, 0).to_uint();
                            break;
                        case 1:
                            m_data = (m_data & 0xffff00ff) |
                                     (ex_mem_data.read().range(7, 0).to_uint() << 8);
                            break;
                        case 2:
                            m_data = (m_data & 0xff00ffff) |
                                     (ex_mem_data.read().range(7, 0).to_uint() << 16);
                            break;
                        case 3:
                            m_data = (m_data & 0x00ffffff) |
                                     (ex_mem_data.read().range(7, 0).to_uint() << 24);
                            break;
                        default:
                            break;
                    }
                    data_for_log     = ex_mem_data.read().range(7, 0).to_uint();
                    addr_sel_for_log = "byte";
                    break;
                case ADDR_HALF_WORD:  // half word access，not necessary aligned to 2
                    switch (byte_sel) {
                        case 0:
                            m_data =
                              (m_data & 0xffff0000) | ex_mem_data.read().range(15, 0).to_uint();
                            break;
                        case 1:
                            m_data = (m_data & 0xff0000ff) |
                                     (ex_mem_data.read().range(15, 0).to_uint() << 8);
                            break;
                        case 2:
                            m_data = (m_data & 0x0000ffff) |
                                     (ex_mem_data.read().range(15, 0).to_uint() << 16);
                            break;
                        case 3:
                            m_data = (m_data & 0x00ffffff) |
                                     (ex_mem_data.read().range(7, 0).to_uint() << 24);
                            m_data_next = (m_data_next & 0xffffff00) |
                                          ex_mem_data.read().range(15, 8).to_uint();
                            break;
                        default:
                            break;
                    }
                    data_for_log     = ex_mem_data.read().range(15, 0).to_uint();
                    addr_sel_for_log = "half";
                    break;
                case ADDR_WORD:  // word access, not necessary aligned to 4
                    switch (byte_sel) {
                        case 0:
                            m_data = ex_mem_data.read().to_uint();
                            break;
                        case 1:
                            m_data = (m_data & 0x000000ff) |
                                     (ex_mem_data.read().range(23, 0).to_uint() << 8);
                            m_data_next = (m_data_next & 0xffffff00) |
                                          ex_mem_data.read().range(31, 24).to_uint();
                            break;
                        case 2:
                            m_data = (m_data & 0x0000ffff) |
                                     (ex_mem_data.read().range(15, 0).to_uint() << 16);
                            m_data_next = (m_data_next & 0xffff0000) |
                                          ex_mem_data.read().range(31, 16).to_uint();
                            break;
                        case 3:
                            m_data = (m_data & 0x00ffffff) |
                                     (ex_mem_data.read().range(7, 0).to_uint() << 24);
                            m_data_next = (m_data_next & 0xff000000) |
                                          ex_mem_data.read().range(31, 8).to_uint();
                            break;
                        default:
                            break;
                    }
                    data_for_log     = ex_mem_data.read().to_uint();
                    addr_sel_for_log = "word";
                    break;
                /*
                case ADDR_HALF_WORD:  // half word access，need addr aligned to 2
                    if ((ex_rd_value.read() & 0x1) != 0) {
                        // not 2 aligned
                        logger->error(
                          "{}: memory write address (0x{:8x}) is not aligned to 2 when write "
                          "half "
                          "word. Simulation aborts!",
                          this->name(), ex_rd_value.read());
                        exit(EXIT_FAILURE);
                    }

                    // upper 16 bit
                    if ((ex_rd_value.read() & 0x2) != 0) {
                        m_data =
                          (m_data & 0x0000ffff) | (ex_mem_data.read().range(15, 0).to_uint() << 16);
                    } else {
                        // lower 16 bit
                        m_data = (m_data & 0xffff0000) | ex_mem_data.read().range(15, 0).to_uint();
                    }
                    data_for_log     = ex_mem_data.read().range(15, 0).to_uint();
                    addr_sel_for_log = "half";
                    break;
                case ADDR_WORD:  // word access, need aligned to 4
                    if ((ex_rd_value.read() & 0x3) != 0) {
                        // not 4 aligned
                        logger->error(
                          "{}: memory write address (0x{:8x}) is not aligned to 4 when write a "
                          "word. Simulation aborts!",
                          this->name(), ex_rd_value.read());
                        exit(EXIT_FAILURE);
                    }
                    m_data           = ex_mem_data.read().to_uint();
                    data_for_log     = ex_mem_data.read().to_uint();
                    addr_sel_for_log = "word";
                    break;
                */
                default:
                    logger->error(
                      "{}: Unrecognized memory access type. Support types: byte, half word, word. "
                      "Simulation aborts!",
                      this->name());
                    exit(EXIT_FAILURE);
                    break;
            }

            // write to mem
            m_data_mem->write_mem(write_addr >> 2, m_data);
            if (need_data_next) {
                m_data_mem->write_mem((write_addr >> 2) + 1, m_data_next);
            }

            // logging
            if (is_telf_on) {
                telf_os << std::setfill(' ') << std::setw(15)
                        << sc_core::sc_time_stamp().to_string();
                telf_os << " " << std::setfill(' ') << std::setw(11) << std::dec << m_num_cycles;
                telf_os << " " << std::setfill(' ') << std::setw(3) << "w";
                telf_os << " " << std::setfill(' ') << std::setw(10) << addr_sel_for_log;
                telf_os << " " << std::setfill('0') << std::setw(8) << std::hex << write_addr;
                telf_os << " " << std::setfill('0') << std::setw(8) << std::hex << data_for_log;
                telf_os << std::endl;
            }
        }
    }
}

void Classical_mem::sign_extend() {
    auto logger = get_logger_or_exit("console");

    sc_int<32> data;

    while (true) {
        wait();

        /* sign extend :  The value of the left-most bit of the data (bit 15 or bit 7) is copied
         * to all bits to the left (into the high-order bits) */
        /* non sign extend: 0 is copied to all bits to the left (into the high-order bits)*/
        switch (mem_addr_sel.read()) {
            case ADDR_BYTE:  // byte access
                if (mem_sext.read() && (mem_out_data.read() & 0x80)) {
                    // sign value and copy 1 to the left bits
                    mem_rd_value.write(mem_out_data.read() | 0xffffff00);
                } else {
                    mem_rd_value.write(mem_out_data.read().to_uint());
                }
                break;
            case ADDR_HALF_WORD:  // half word access
                if (mem_sext.read() && (mem_out_data.read() & 0x8000)) {
                    // sign value and copy 1 to the left bits
                    mem_rd_value.write(mem_out_data.read() | 0xffff0000);
                } else {
                    mem_rd_value.write(mem_out_data.read().to_uint());
                }
                break;
            case ADDR_WORD:  // word access
                mem_rd_value.write(mem_out_data.read().to_uint());
                break;
            default:
                logger->error(
                  "{}: Unrecognized memory access type. Support types: byte, half word, word. "
                  "Simulation aborts!",
                  this->name());
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void Classical_mem::clock_counter() {
    while (true) {
        wait();
        m_num_cycles++;
    }
}

void Classical_mem::add_telf_header() {
    // Specify the signal type
    telf_os << "#memory access" << std::endl;
    telf_os << std::setfill(' ') << std::setw(15) << "sim_time";
    telf_os << " " << std::setfill(' ') << std::setw(11) << "cycle";
    telf_os << " " << std::setfill(' ') << std::setw(3) << "r/w";
    telf_os << " " << std::setfill(' ') << std::setw(10) << "addr_sel";
    telf_os << " " << std::setfill(' ') << std::setw(8) << "address";
    telf_os << " " << std::setfill(' ') << std::setw(8) << "value";
    telf_os << std::endl;
}

}  // namespace cactus
