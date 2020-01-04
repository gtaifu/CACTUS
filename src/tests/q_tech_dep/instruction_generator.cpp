#include "instruction_generator.h"

#include <regex>

namespace cactus {

void Instruction_generator::config() {
    Global_config& global_config = Global_config::get_instance();

    if (global_config.instruction_type == Instruction_type::ASM) {
        read_asm_file(global_config.qisa_asm_fn);
    }

    m_instruction_type = global_config.instruction_type;
}

Instruction_generator::Instruction_generator(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->info("Start initializing {}...", this->name());

    config();

    SC_CTHREAD(output, in_clock.pos());

    logger->info("Successfully initializing {}...", this->name());
}

void Instruction_generator::output() {

    auto logger = get_logger_or_exit("console");

    Qasm_instruction cur_insn;
    unsigned int     imm_wait_time;
    bool             valid;

    unsigned insn_count = 0;

    while (true) {
        wait();

        valid         = out_valid.read();
        cur_insn      = out_bundle.read();
        imm_wait_time = out_rs_wait_time.read().to_uint();

        if (cur_insn.type == Instruction_type::BIN) {
            logger->trace("{}: sim_time:{}, insn_code:{:08X}, imm_wait_time:{:08X}, insn_valid:{}",
                          this->name(), sc_core::sc_time_stamp().to_string(),
                          cur_insn.get_insn_bin(), imm_wait_time, valid);
        } else {
            logger->trace("{}: sim_time:{}, insn_code:{}, imm_wait_time:{:08X}, insn_valid:{}",
                          this->name(), sc_core::sc_time_stamp().to_string(),
                          cur_insn.get_insn_asm(), imm_wait_time, valid);
        }

        if (event_queue_almostfull.read()) {

            out_valid.write(false);

        } else if (m_instruction_type == Instruction_type::BIN) {

            cur_insn.set_instruction(output_insn[insn_count % 10].insn_code, insn_count);
            out_bundle.write(cur_insn);
            out_valid.write(output_insn[insn_count % 10].valid);
            out_rs_wait_time.write(output_insn[insn_count % 10].wait_time);
            insn_count++;

        } else {

            cur_insn.set_instruction(asm_instruction[insn_count % asm_instruction.size()],
                                     map_label, insn_count);
            out_bundle.write(cur_insn);
            out_valid.write(cur_insn.is_q_insn());
            out_rs_wait_time.write(0);
            insn_count++;
        }
    }
}

void Instruction_generator::read_asm_file(const std::string& qisa_asm_fn) {
    auto logger = get_logger_or_exit("console");

    logger->info("{}: Initializing the source asm instructions with the file: {}.", this->name(),
                 qisa_asm_fn);

    asm_instruction.clear();

    std::ifstream qisa_file;
    qisa_file.open(qisa_asm_fn);
    if (!qisa_file) {
        logger->error("{}: Failed to open file: {}. Simulation aborts.", this->name(), qisa_asm_fn);
        exit(EXIT_FAILURE);
    }
    logger->debug("{}: Successfully opened the file: {}.", this->name(), qisa_asm_fn);

    std::string insn;

    while (getline(qisa_file, insn)) {
        std::regex pattern("[0-9a-zA-Z]+");
        if (std::regex_search(insn, pattern)) {
            asm_instruction.push_back(trim(insn));
        }
    }

    for (size_t i = 0; i < 5; ++i) {  // add 5 extra stop instructions
        asm_instruction.push_back("stop");
    }

    logger->info("{}: Successfully read the asm qisa program. ", this->name());

    qisa_file.close();
}

}  // namespace cactus
