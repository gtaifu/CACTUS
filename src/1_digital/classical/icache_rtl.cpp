#include "icache_rtl.h"

#include <fstream>
#include <iomanip>
#include <regex>  //regular expression
#include <sstream>
#include <systemc>

#include "logger_wrapper.h"

namespace cactus {

void Icache_rtl::init_mem_bin(std::string qisa_bin_fn) {

    auto logger = get_logger_or_exit("cache_logger");

    logger->trace("{}: Initializing the ICACHE with the binary file: '{}'.", this->name(),
                  qisa_bin_fn);

    cache_mem_bin.clear();
    cache_mem_bin.resize(CACHE_SIZE);

    std::ifstream qisa_file;

    qisa_file.open(qisa_bin_fn, std::ios::binary);

    if (!qisa_file) {
        logger->error("{}: Failed to open file: '{}'. Simulation aborts!", this->name(),
                      qisa_bin_fn);
        exit(EXIT_FAILURE);
    }
    logger->debug("{}: Successfully opened the file '{}'.", this->name(), qisa_bin_fn);

    qisa_file.seekg(0, qisa_file.end);
    unsigned int length = static_cast<unsigned int>(qisa_file.tellg());

    if (length / 4 > CACHE_SIZE) {
        logger->error(
          "{}: The size of the input program ({}) exceeds the cache size ({}). "
          "Simulation aborts!",
          this->name(), length / 4, CACHE_SIZE);
        exit(EXIT_FAILURE);
    }
    logger->debug(
      "{}: The size of the input program ({}) does not exceed the"
      " cache size ({}).",
      this->name(), length / 4, CACHE_SIZE);

    program_length = length / 4;

    qisa_file.seekg(0, qisa_file.beg);

    // every instruction is 32 bits. The additional 4 is a slight larger space to avoid bug.
    char* buffer = new char[CACHE_SIZE * 4 + 4];

    qisa_file.read(buffer, length);

    logger->debug("{}: Successfully read the binary file into the buffer (size: {}).", this->name(),
                  length);
    unsigned int cur_insn;

    std::stringstream ss;

    ss.str("");
    for (size_t i = 0; i < length; i = i + 4) {
        cur_insn = (buffer[i] & 0xFF) + ((buffer[i + 1] << 8) & 0xFF00) +
                   ((buffer[i + 2] << 16) & 0xFF0000) + ((buffer[i + 3] << 24) & 0xFF000000);

        cache_mem_bin[i / 4] = cur_insn;

        ss << "0x" << std::setfill('0') << std::setw(8) << std::hex << cur_insn << "  ";

        if ((i + 4) % 32 == 0) {
            ss << std::endl;
        }
    }
    delete[] buffer;

    logger->debug("{}: Instructions read from the file:\n{}", this->name(), ss.str());

    logger->trace("{}: Successfully read the binary qisa program, which has {} instructions.",
                  this->name(), length / 4);

    qisa_file.close();
}

unsigned int Icache_rtl::convert_line_to_ele_instr(std::vector<std::vector<std::string>>& vec_instr,
                                                   std::string&                           line_str,
                                                   const std::string& line_num_str) {
    std::vector<std::string> instr_info;
    std::string              bne_content = "";
    std::string              br_label    = "";
    std::string              br_cond     = "";

    // find macro bne,beq
    trim(line_str);
    std::regex pattern_bne("bne", std::regex_constants::icase);
    std::regex pattern_beq("beq", std::regex_constants::icase);
    if (std::regex_search(line_str, pattern_bne)) {
        br_cond = "ne";
    } else if (std::regex_search(line_str, pattern_beq)) {
        br_cond = "eq";
    } else {
        // not beq or bne
        instr_info.push_back(line_str);
        instr_info.push_back(line_num_str);
        instr_info.push_back(line_str);
        vec_instr.push_back(instr_info);
        return 1;
    }

    size_t start = line_str.find_first_of(" ");
    if (start != line_str.npos) {
        bne_content = line_str.substr(start);
    }
    size_t end = bne_content.find_last_of(",");
    if (end != bne_content.npos) {
        br_label = bne_content.substr(end);
        bne_content.erase(end);
    }
    // push back cmp
    instr_info.push_back("cmp " + bne_content);
    instr_info.push_back(line_num_str);
    instr_info.push_back(line_str);
    vec_instr.push_back(instr_info);

    // push back nop
    instr_info.clear();
    instr_info.push_back("nop");
    instr_info.push_back(line_num_str);
    instr_info.push_back(line_str);
    vec_instr.push_back(instr_info);

    // push back br
    instr_info.clear();
    instr_info.push_back("br " + br_cond + br_label);
    instr_info.push_back(line_num_str);
    instr_info.push_back(line_str);
    vec_instr.push_back(instr_info);
    return 3;
}

void Icache_rtl::init_mem_asm(std::string qisa_asm_fn) {
    auto logger = get_logger_or_exit("cache_logger");

    logger->trace("{}: Initializing the ICACHE with the asm file: '{}'.", this->name(),
                  qisa_asm_fn);

    cache_mem_asm.clear();

    std::ifstream qisa_file;
    qisa_file.open(qisa_asm_fn);
    if (!qisa_file) {
        logger->error("{}: Failed to open file: '{}'. Simulation aborts!", this->name(),
                      qisa_asm_fn);
        exit(EXIT_FAILURE);
    }
    logger->debug("{}: Successfully opened the file '{}'.", this->name(), qisa_asm_fn);

    unsigned int count = 0;
    std::string  insn;
    unsigned int line_num = 1;
    std::string  line_num_str;

    while (getline(qisa_file, insn)) {

        // remove comments which indicate by first char '#'
        line_num_str = to_string(line_num++);
        trim_comments(insn);

        // ex: "label: add r1,r2,r3"
        if (insn.find(":") != insn.npos) {
            std::vector<std::string> sub_parts;
            sub_parts = split(insn, ':');

            std::string label = trim(sub_parts[0]);
            map_label.insert(std::make_pair(label, count));

            std::regex pattern("[0-9a-zA-Z]+");
            if ((sub_parts.size() == 2) && std::regex_search(sub_parts[1], pattern)) {
                // cache_mem_asm.push_back(trim(sub_parts[1]));
                // count++;
                count += convert_line_to_ele_instr(cache_mem_asm, sub_parts[1], line_num_str);
            }
        } else {
            // ex："add r1,r2,r3"
            std::regex pattern("[0-9a-zA-Z]+");
            if (std::regex_search(insn, pattern)) {
                // cache_mem_asm.push_back(trim(insn));
                // count++;
                count += convert_line_to_ele_instr(cache_mem_asm, insn, line_num_str);
            }
        }
    }

    std::vector<std::string> stop_instr;
    stop_instr.push_back("stop");
    stop_instr.push_back(to_string(line_num++));
    stop_instr.push_back("stop");
    cache_mem_asm.push_back(stop_instr);  // add extra stop instructions at the end

    logger->trace("{}: Successfully read the asm qisa program, which has {} instructions.",
                  this->name(), count);

    qisa_file.close();
}

void Icache_rtl::combinational_gen() {
    auto logger = get_logger_or_exit("cache_logger");

    while (true) {
        wait();

        if (Clp2Ic_branching.read() && Clp2Ic_ready.read()) {
            // logger->debug("Clp2Ic_branching is 1. Jump. Target: {}.",
            // static_cast<unsigned int>(Clp2Ic_target.read()));
            pc = Clp2Ic_target.read();
            branch.write(1);
        } else {
            // logger->debug("No jump. Next PC: {}.",
            // static_cast<unsigned int>(pcc.read()));
            pc = pcc.read();
            branch.write(branchc.read());
        }

        if (readya.read()) {
            pcb = pca.read() + 1;
            // logger->debug("ReadyA is 1. PC_B: {}.", static_cast<unsigned int>(pcb.read()));
            branchb.write(0);
        } else {
            pcb = pca.read();
            // logger->debug("ReadyA is 0. PC_B: {}.", static_cast<unsigned int>(pcb.read()));
            branchb = brancha.read();
        }
    }
}

void Icache_rtl::register_right() {
    auto logger = get_logger_or_exit("cache_logger");
    while (true) {
        wait();

        // logger->debug("Called the function register_right().");

        if (G_PC_REGISTER > 0) {
            pca     = pc.read();
            brancha = branch.read();
            // logger->debug("Read readya: {} with G_PC_REGISTER > 0.",
            //     static_cast<unsigned int>(Clp2Ic_ready.read()));
            readya = Clp2Ic_ready.read();
        }
    }
}

void Icache_rtl::register_left_logic() {
    auto logger = get_logger_or_exit("cache_logger");

    while (true) {
        wait();

        // logger->debug("Called the function register_left_logic().");

        if (G_PC_REGISTER == 0) {
            pca     = pc.read();
            brancha = branch.read();
            // logger->debug("Read readya: {} with G_PC_REGISTER = 0.",
            //     static_cast<unsigned int>(Clp2Ic_ready.read()));
            readya = Clp2Ic_ready.read();

            if (reset.read()) {
                brancha.write(0);
            }
        }
    }
}

void Icache_rtl::register_right_logic() {
    while (true) {
        wait();

        if (G_PC_REGISTER > 0) {
            pcc     = pcb.read();
            branchc = branchb.read();

            if (reset.read()) {
                branchc.write(0);
            }
        }
    }
}

void Icache_rtl::register_at_memory_pipeline() {
    while (true) {
        wait();

        if (G_PC_REGISTER == 2) {
            if (Clp2Ic_ready.read()) {
                pc_reg_a     = pc.read();
                branch_reg_a = branch.read();
            }
            if (reset.read()) {
                branch_reg_a.write(0);
            }
        }
    }
}

void Icache_rtl::no_register_memory_pipeline() {
    while (true) {
        wait();

        if (G_PC_REGISTER < 2) {
            pc_reg_a     = pc.read();
            branch_reg_a = branch.read();
        }
    }
}

void Icache_rtl::read_insn() {

    auto logger = get_logger_or_exit("cache_logger");

    unsigned int     cache_pc;
    Qasm_instruction v_insn;

    while (true) {
        wait();

        // the Branch signal needs to go to the processor one transfer earlier than the instruction
        // signal.
        if (Clp2Ic_ready.read()) {
            cache_pc = pc_reg_a.read().to_uint();
            logger->debug("{}: trying to read. PC to read: 0x{:08x}", this->name(), cache_pc);
            if (m_instruction_type == Instruction_type::BIN) {
                v_insn.set_instruction(cache_mem_bin[cache_pc], cache_pc);
            } else {
                if (cache_pc >= cache_mem_asm.size()) {
                    v_insn.set_instruction(cache_mem_asm[cache_mem_asm.size() - 1], map_label,
                                           cache_pc);
                } else {
                    v_insn.set_instruction(cache_mem_asm[cache_pc], map_label, cache_pc);
                }
            }

            if (G_MEM_OUT_REG != 0) {
                insn_reg_b = v_insn;
                insn_reg_c = insn_reg_b.read();
            } else
                insn_reg_c = v_insn;
        }
    }
}

void Icache_rtl::memory_out_reg() {
    while (true) {
        wait();

        if (G_MEM_OUT_REG != 0) {
            if (Clp2Ic_ready.read()) {
                branch_reg_b = branch_reg_a.read();
            }
            if (reset.read()) {
                branch_reg_b.write(0);
            }
        }
    }
}

void Icache_rtl::no_memory_out_reg() {
    while (true) {
        wait();

        if (G_MEM_OUT_REG == 0) {
            branch_reg_b = branch_reg_a.read();
        }
    }
}

void Icache_rtl::drive_cache_output() {
    while (true) {
        wait();

        IC2Clp_insn    = insn_reg_c.read();
        IC2Clp_br_done = branch_reg_b.read();
        // The output is always valid since this is an on-chip memory
        IC2Clp_valid.write(1);
    }
}

void Icache_rtl::clock_counter() {
    while (true) {
        wait();
        num_cycles++;
    }
}

// void Icache_rtl::write_output_file() {
// 	sc_uint<INSN_WIDTH>								v_insn;
// 	sc_uint<MEMORY_ADDRESS_WIDTH>                   reg_pc;
// 	while (true) {
// 		wait();

// 		if (Clp2Ic_ready.read()) {
// 			if (reg_pc != pc_reg_a.read()) {
// 				reg_pc = pc_reg_a.read();
// 			}
// 			v_insn = cache_mem[static_cast<size_t>(reg_pc)];
// 		}

// 		if (Clp2Ic_ready.read()) {
// 			if (reg_pc < program_length) {
// 				//Write the text output
// 				insn_result_veri_out << std::setfill(' ') << std::setw(11) <<
// std::dec
// <<
// num_cycles; 				insn_result_veri_out << ",    " << std::setfill(' ') <<
// std::setw(8) << pc_reg_a.read(); 				insn_result_veri_out << ",    " <<
// std::setfill(' ') << std::setw(11) << branch_reg_a.read();
// insn_result_veri_out << ",    " << std::setfill(' ') << std::setw(4) << "0x'" << std::hex <<
// v_insn; 				insn_result_veri_out << std::endl;
// 			}
// 		}
// 	}
// }
}  // namespace cactus
