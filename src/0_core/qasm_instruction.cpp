#include "qasm_instruction.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <regex>  // regular expression
#include <sstream>

#include "num_util.h"

namespace cactus {

Instruction_type Qasm_instruction::get_type() { return type; }

unsigned int Qasm_instruction::get_insn_bin() { return insn_bin; }

std::string Qasm_instruction::get_insn_asm() { return insn_asm; }

std::string Qasm_instruction::get_insn_line_num_in_file() { return insn_line_num_in_file; }

std::string Qasm_instruction::get_insn_str_in_file() { return insn_str_in_file; }

unsigned int Qasm_instruction::get_opcode() { return opcode; }

unsigned int Qasm_instruction::get_rs_addr() { return rs_addr; }

unsigned int Qasm_instruction::get_rt_addr() { return rt_addr; }

unsigned int Qasm_instruction::get_rd_addr() { return rd_addr; }

unsigned int Qasm_instruction::get_uimm() { return uimm; }

int Qasm_instruction::get_imm() { return imm; }

int Qasm_instruction::get_br_addr() { return br_addr; }

unsigned int Qasm_instruction::get_br_cond() { return br_cond; }

unsigned int Qasm_instruction::get_qubit_sel() { return qubit_sel; }

Q_instr_type Qasm_instruction::get_q_insn_type() { return q_insn_type; }

unsigned int Qasm_instruction::get_q_time_specified() { return q_time_specified; }

std::vector<size_t>& Qasm_instruction::get_q_qubit_indices() { return q_qubit_indices; }

std::vector<std::vector<size_t>>& Qasm_instruction::get_q_qubit_tuples() { return q_qubit_tuples; }

std::vector<unsigned int>& Qasm_instruction::get_q_reg_num() { return q_reg_num; }

std::vector<std::string>&           Qasm_instruction::get_q_op_name() { return q_op_name; }
std::vector<num_tgt_qubits_type_t>& Qasm_instruction::get_q_num_tgt_qubits_type() {
    return q_num_tgt_qubits_type;
}

bool Qasm_instruction::is_q_insn() { return q_insn; }

bool Qasm_instruction::is_rd_used() { return rd_used; }

bool Qasm_instruction::is_rs_used() { return rs_used; }

bool Qasm_instruction::is_rt_used() { return rt_used; }

bool Qasm_instruction::is_cl_insn() { return cl_insn; }

bool Qasm_instruction::is_meas() { return meas_insn; }

bool Qasm_instruction::is_fmr() { return cl_insn && (opcode == OperationName::FMR); }

bool Qasm_instruction::is_br() { return cl_insn && (opcode == OperationName::BR); }

bool Qasm_instruction::is_stop() { return cl_insn && (opcode == OperationName::STOP); }

void Qasm_instruction::parse_opcode(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern_stop("stop", std::regex_constants::icase);
    std::regex pattern_nop("^\\s*nop\\s*$", std::regex_constants::icase);
    std::regex pattern_meas("meas", std::regex_constants::icase);

    if (insn.find(" ") != insn.npos) {
        std::string op_string = insn.substr(0, insn.find_first_of(" "));
        trim(op_string);

        // transform op_string to lower case
        transform(op_string.begin(), op_string.end(), op_string.begin(), ::tolower);

        auto it = map_opcode.find(op_string);
        if (it != map_opcode.end()) {
            cl_insn   = true;
            q_insn    = false;
            meas_insn = false;
            opcode    = it->second;
        } else {
            cl_insn   = false;
            q_insn    = true;
            meas_insn = false;
            opcode    = OperationName::NOP;
        }
    } else if (std::regex_search(insn, pattern_nop)) {
        cl_insn   = true;
        q_insn    = false;
        meas_insn = false;
        opcode    = OperationName::NOP;
    } else if (std::regex_search(insn, pattern_stop)) {
        cl_insn   = true;
        q_insn    = false;
        meas_insn = false;
        opcode    = OperationName::STOP;
    } else {
        cl_insn   = false;
        q_insn    = true;
        meas_insn = false;
        opcode    = OperationName::NOP;
    }

    if (std::regex_search(insn, pattern_meas)) {
        cl_insn   = false;
        q_insn    = true;
        meas_insn = true;
        opcode    = OperationName::NOP;
    }
}

// br <br_cond>,<label>
void Qasm_instruction::parse_br(const std::string&                         insn,
                                const std::map<std::string, unsigned int>& map_label,
                                const unsigned int&                        addr) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string br_content;  // <br_cond>,<label>
    if (insn.find(" ") != insn.npos) {
        br_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> sub_parts;  // <br_cond> and <label>
    sub_parts = split(br_content, ',');
    if (sub_parts.size() != 2) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    // get br condition
    std::string br_cond_str = sub_parts[0];
    trim(br_cond_str);

    // transform to lower case
    transform(br_cond_str.begin(), br_cond_str.end(), br_cond_str.begin(), ::tolower);

    auto it_cond = map_br_cond.find(br_cond_str);
    if (it_cond != map_br_cond.end()) {
        br_cond = it_cond->second;
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    // get branch label
    std::string  label_str = sub_parts[1];
    unsigned int label_addr;
    trim(label_str);
    auto it_label = map_label.find(label_str);
    if (it_label != map_label.end()) {
        label_addr = it_label->second;
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    br_addr = label_addr - addr;
}

// cmp rs,rt
void Qasm_instruction::parse_cmp(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 2) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;

    it++;
    rt_addr = str_to_uint(it->str());  // rt_addr
    rt_used = true;
}

// fbr <br_cond>,rd
void Qasm_instruction::parse_fbr(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string fbr_content;  // <br_cond>,<label>
    if (insn.find(" ") != insn.npos) {
        fbr_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> sub_parts;  // <br_cond> and rd
    sub_parts = split(fbr_content, ',');
    if (sub_parts.size() != 2) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    // get br condition
    std::string fbr_cond_str = sub_parts[0];
    trim(fbr_cond_str);

    // transform to lower case
    transform(fbr_cond_str.begin(), fbr_cond_str.end(), fbr_cond_str.begin(), ::tolower);

    auto it_cond = map_br_cond.find(fbr_cond_str);
    if (it_cond != map_br_cond.end()) {
        br_cond = it_cond->second;
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    // get rd_addr
    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 1) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());
    rd_used = true;
}

// fmr rd,Qi
void Qasm_instruction::parse_fmr(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 2) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    qubit_sel = str_to_uint(it->str());  // qubit_sel
}

// ldi rd,imm
void Qasm_instruction::parse_ldi(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string ldi_content;
    if (insn.find(" ") != insn.npos) {
        ldi_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("-?(0[xX])?[0-9A-Fa-f]+");
    auto       begin = std::sregex_iterator(ldi_content.begin(), ldi_content.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 2) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    std::string imm_str = it->str();
    if ((imm_str.find("0X") != imm_str.npos) || (imm_str.find("0x") != imm_str.npos)) {
        imm = hexstr_to_int(imm_str);
    } else {
        imm = str_to_int(imm_str);  // imm
    }
}

// ldui rd,rs,imm
void Qasm_instruction::parse_ldui(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string ldui_content;
    if (insn.find(" ") != insn.npos) {
        ldui_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("(0[xX])?[0-9A-Fa-f]+");
    auto       begin = std::sregex_iterator(ldui_content.begin(), ldui_content.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 3) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;

    it++;
    std::string uimm_str = it->str();
    if ((uimm_str.find("0X") != uimm_str.npos) || (uimm_str.find("0x") != uimm_str.npos)) {
        uimm = hexstr_to_uint(uimm_str);
    } else {
        uimm = str_to_uint(uimm_str);  // imm
    }
}

// lb rd,offset(rs)
// lbu rd,offset(rs)
// lw rd,offset(rs)
void Qasm_instruction::parse_load_mem(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string load_content;
    if (insn.find(" ") != insn.npos) {
        load_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("-?(0[xX])?[0-9A-Fa-f]+");
    auto       begin = std::sregex_iterator(load_content.begin(), load_content.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 3) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    std::string imm_str = it->str();
    if ((imm_str.find("0X") != imm_str.npos) || (imm_str.find("0x") != imm_str.npos)) {
        imm = hexstr_to_int(imm_str);
    } else {
        imm = str_to_int(imm_str);  // imm
    }

    it++;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;
}

// sb rt,offset(rs)
// sw rt,offset(rs)
void Qasm_instruction::parse_store_mem(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string store_content;
    if (insn.find(" ") != insn.npos) {
        store_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("-?(0[xX])?[0-9A-Fa-f]+");
    auto       begin = std::sregex_iterator(store_content.begin(), store_content.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 3) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rt_addr = str_to_uint(it->str());  // rt_addr
    rt_used = true;

    it++;
    std::string imm_str = it->str();
    if ((imm_str.find("0X") != imm_str.npos) || (imm_str.find("0x") != imm_str.npos)) {
        imm = hexstr_to_int(imm_str);
    } else {
        imm = str_to_int(imm_str);  // imm
    }

    it++;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;
}

// or rd,rs,rt
// and rd,rs,rt
// xor rd,rs,rt
void Qasm_instruction::parse_logic_operation(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 3) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;

    it++;
    rt_addr = str_to_uint(it->str());  // rt_addr
    rt_used = true;
}

// not rd,rt
void Qasm_instruction::parse_not(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 2) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    rt_addr = str_to_uint(it->str());  // rt_addr
    rt_used = true;
}

// add rd,rs,rt
// sub rd,rs,rt
// rem rd,rs,rt
// mul rd,rs,rt
// div rd,rs,rt
void Qasm_instruction::parse_arithmetic_operation(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 3) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;

    it++;
    rt_addr = str_to_uint(it->str());  // rt_addr
    rt_used = true;
}

// addi rd,rs,imm
void Qasm_instruction::parse_arithmetic_immediate_operation(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::string arith_content;
    if (insn.find(" ") != insn.npos) {
        arith_content = insn.substr(insn.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("-?(0[xX])?[0-9A-Fa-f]+");
    auto       begin = std::sregex_iterator(arith_content.begin(), arith_content.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 3) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rd_addr = str_to_uint(it->str());  // rd_addr
    rd_used = true;

    it++;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;

    it++;
    std::string imm_str = it->str();  // imm
    if ((imm_str.find("0X") != imm_str.npos) || (imm_str.find("0x") != imm_str.npos)) {
        imm = hexstr_to_int(imm_str);
    } else {
        imm = str_to_int(imm_str);
    }
}

// qwaitr r1
void Qasm_instruction::parse_qwaitr(const std::string& insn) {
    auto logger = get_logger_or_exit("asm_logger");

    std::regex pattern_qwaitr("qwaitr", std::regex_constants::icase);

    if (!std::regex_search(insn, pattern_qwaitr)) {
        return;
    }

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn.begin(), insn.end(), pattern);
    auto       end   = std::sregex_iterator();
    if (std::distance(begin, end) != 1) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    rs_addr = str_to_uint(it->str());  // rs_addr
    rs_used = true;
}

void Qasm_instruction::parse_q_insn() {

    // parse q instr type
    parse_q_insn_type();

    switch (q_insn_type) {
        case Q_instr_type::Q_WAIT:
            parse_qwait();
            break;
        case Q_instr_type::Q_SMIS:
            parse_smis();
            break;
        case Q_instr_type::Q_SMIT:
            parse_smit();
            break;
        case Q_instr_type::Q_OP:
            parse_qop();
            break;

        default:
            break;
    }
}

void Qasm_instruction::parse_q_insn_type() {

    std::regex pattern_nop("^\\s*nop\\s*$", std::regex_constants::icase);
    std::regex pattern_stop("stop", std::regex_constants::icase);
    std::regex pattern_qwaitr("qwaitr", std::regex_constants::icase);
    std::regex pattern_qwait("qwait", std::regex_constants::icase);
    std::regex pattern_smit("smit", std::regex_constants::icase);
    std::regex pattern_smis("smis", std::regex_constants::icase);

    // operation type
    if (std::regex_search(insn_asm, pattern_smis)) {

        q_insn_type = Q_instr_type::Q_SMIS;

    } else if (std::regex_search(insn_asm, pattern_smit)) {

        q_insn_type = Q_instr_type::Q_SMIT;

    } else if (std::regex_search(insn_asm, pattern_qwaitr)) {

        q_insn_type = Q_instr_type::Q_WAITR;

    } else if (std::regex_search(insn_asm, pattern_qwait)) {

        q_insn_type = Q_instr_type::Q_WAIT;

    } else if (std::regex_search(insn_asm, pattern_stop)) {

        q_insn_type = Q_instr_type::Q_STOP;

    } else if (std::regex_search(insn_asm, pattern_nop)) {

        q_insn_type = Q_instr_type::Q_NOP;

    } else {
        q_insn_type = Q_instr_type::Q_OP;  // valid operation
    }
}

void Qasm_instruction::parse_qwait() {

    auto logger = get_logger_or_exit("asm_logger");

    std::string wait_content;
    if (insn_asm.find(" ") != insn_asm.npos) {
        wait_content = insn_asm.substr(insn_asm.find(" "));
    } else {
        logger->error(
          "asm_parser: Cannot parse qwait instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("(0[xX])?[0-9A-Fa-f]+");
    auto       begin = std::sregex_iterator(wait_content.begin(), wait_content.end(), pattern);
    auto       end   = std::sregex_iterator();
    auto       dist  = std::distance(begin, end);
    if (dist != 1) {
        logger->error(
          "asm_parser: Cannot parse qwait instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto        it            = begin;
    std::string wait_time_str = it->str();
    if ((wait_time_str.find("0X") != wait_time_str.npos) ||
        (wait_time_str.find("0x") != wait_time_str.npos)) {
        q_time_specified = hexstr_to_uint(wait_time_str);
    } else {
        q_time_specified = str_to_uint(wait_time_str);
    }
}

void Qasm_instruction::parse_smis() {
    auto logger = get_logger_or_exit("asm_logger");

    // evite smis instr with T type register
    std::regex pattern_error("t\\d+");
    if (std::regex_search(insn_asm, pattern_error)) {
        logger->error(
          "asm_parser: Cannot parse smis instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn_asm.begin(), insn_asm.end(), pattern);
    auto       end   = std::sregex_iterator();
    auto       dist  = std::distance(begin, end);
    if (dist < 2) {
        logger->error(
          "asm_parser: Cannot parse smis instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    q_reg_num.push_back(str_to_uint(it->str()));

    for (++it; it != end; ++it) {
        q_qubit_indices.push_back(str_to_sizet(it->str()));
    }
}

void Qasm_instruction::parse_smit() {
    auto logger = get_logger_or_exit("asm_logger");

    // evite smis instr with S type register
    std::regex pattern_error("s\\d+");
    if (std::regex_search(insn_asm, pattern_error)) {
        logger->error(
          "asm_parser: Cannot parse smit instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::regex pattern("\\d+");
    auto       begin = std::sregex_iterator(insn_asm.begin(), insn_asm.end(), pattern);
    auto       end   = std::sregex_iterator();
    auto       dist  = std::distance(begin, end);
    if (((dist % 2) == 0) || (dist < 3)) {
        logger->error(
          "asm_parser: Cannot parse smit instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    auto it = begin;
    q_reg_num.push_back(str_to_uint(it->str()));

    std::vector<size_t> pair;
    for (++it; it != end; ++it) {
        pair.clear();  // clear pair

        pair.push_back(str_to_sizet(it->str()));  // left qubit

        ++it;
        pair.push_back(str_to_sizet(it->str()));  // right qubit

        q_qubit_tuples.push_back(pair);  // add to qubit tuples
    }
}

void Qasm_instruction::verify_rotate_angle(const std::string op_name, std::string& op_name_prefix) {

    auto logger = get_logger_or_exit("asm_logger");

    // to verify whole name
    std::regex pattern_rotate("^r?[xyz]m?\\d+(_\\d+)?$", std::regex_constants::icase);
    if (!std::regex_search(op_name, pattern_rotate)) {
        logger->error(
          "asm_parser: Cannot parse smit instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    // to verify r[xyz]m
    std::regex pattern_rxm("^r[xyz]m", std::regex_constants::icase);
    if (std::regex_search(op_name, pattern_rxm)) {
        logger->error(
          "asm_parser: Cannot parse smit instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    // to get angle
    std::regex pattern_angle("\\d{1,3}(_\\d+)?$");
    auto       begin = std::sregex_iterator(op_name.begin(), op_name.end(), pattern_angle);
    auto       end   = std::sregex_iterator();
    auto       dist  = std::distance(begin, end);
    if (dist != 1) {
        logger->error(
          "asm_parser: Cannot parse smit instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }

    std::string angle_str = begin->str();
    if (angle_str.find("_") != angle_str.npos) {
        angle_str[angle_str.find("_")] = '.';
    }
    double angle = std::stod(angle_str, nullptr);
    if (angle > 180) {
        logger->error(
          "asm_parser: Cannot parse smit instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }
    // before verify rotate angle,it has been checked that its size is at least 2
    if ((op_name[0] == 'r') || (op_name[1] == 'm')) {
        op_name_prefix = op_name.substr(0, 2);
    } else {
        op_name_prefix = op_name.substr(0, 1);
    }
}

void Qasm_instruction::parse_qop() {

    auto logger = get_logger_or_exit("asm_logger");

    Global_config& global_config = Global_config::get_instance();

    // remove wait time
    std::string qop_content;
    std::regex  pattern_specified_time("^\\d+\\s*,\\s*\\w", std::regex_constants::icase);
    if (std::regex_search(insn_asm, pattern_specified_time)) {
        std::regex  pattern_time("\\d+");
        std::smatch results;
        std::regex_search(insn_asm, results, pattern_time);
        auto it          = results.begin();
        q_time_specified = str_to_uint(it->str());

        qop_content = insn_asm.substr(insn_asm.find_first_of(",") + 1);
    } else {

        q_time_specified = 1;  // default 1 when there is no specified time
        qop_content      = insn_asm;
    }

    // parser qop
    trim(qop_content);
    std::vector<std::string> qops;
    qops = split(qop_content, '|');  // split instruction with delimiter |

    bool is_mock_meas = false;
    for (auto it = qops.begin(); it != qops.end(); ++it) {
        std::string op = *it;
        trim(op);

        // tranform op name to lower case
        transform(op.begin(), op.end(), op.begin(), ::tolower);

        // get operation name
        std::string  op_name;
        unsigned int reg_num;

        if (op != "mock_meas") {
            if (op.find_first_of(" ") == op.npos) {
                logger->error(
                  "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
                  insn_str_in_file, insn_line_num_in_file);
                exit(EXIT_FAILURE);
            }
            op_name = op.substr(0, op.find_first_of(" "));

            std::regex pattern_meas("meas", std::regex_constants::icase);
            if (std::regex_search(op_name, pattern_meas)) {
                op_name = "measure";
            }

            std::regex  pattern_reg("\\d+");
            std::string reg_str = op.substr(op.find_first_of(" "));
            auto        begin   = std::sregex_iterator(reg_str.begin(), reg_str.end(), pattern_reg);
            auto        end     = std::sregex_iterator();
            auto        dist    = std::distance(begin, end);
            if (dist != 1) {
                logger->error(
                  "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
                  insn_str_in_file, insn_line_num_in_file);
                exit(EXIT_FAILURE);
            }
            reg_num = str_to_uint(begin->str());
        } else {
            op_name      = op;
            reg_num      = 0;
            is_mock_meas = true;
        }

        // find the original name in configure file
        std::string op_name_prefix;

        // specify measure operation name to 'meas'
        std::regex pattern_rotate("^r?[xyz](m)?", std::regex_constants::icase);
        if (op_name.size() > 1 && std::regex_search(op_name, pattern_rotate)) {
            verify_rotate_angle(op_name, op_name_prefix);
        } else {
            op_name_prefix = op_name;
        }

        // find the matched qubit gate and num qubits target type
        num_tgt_qubits_type_t num_tgt_qubits_ype;
        bool                  get_num_tgt_qubits_type = false;
        auto                  s_it = global_config.single_qubit_gate_time.find(op_name_prefix);
        if (s_it != global_config.single_qubit_gate_time.end()) {
            get_num_tgt_qubits_type = true;
            num_tgt_qubits_ype      = num_tgt_qubits_type_t::SINGLE;
        }

        auto t_it = global_config.two_qubit_gate_time.find(op_name_prefix);
        if (t_it != global_config.two_qubit_gate_time.end()) {
            get_num_tgt_qubits_type = true;
            num_tgt_qubits_ype      = num_tgt_qubits_type_t::MULTIPLE;
        }

        if (!get_num_tgt_qubits_type) {
            logger->error(
              "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
              insn_str_in_file, insn_line_num_in_file);
            exit(EXIT_FAILURE);
        }

        // store op name and register
        q_op_name.push_back(op_name);
        q_reg_num.push_back(reg_num);
        q_num_tgt_qubits_type.push_back(num_tgt_qubits_ype);
    }

    if ((q_op_name.size() == 0) || (is_mock_meas && (q_op_name.size() > 1))) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }
}

unsigned int Qasm_instruction::verify_parser_result() {

    Global_config& global_config = Global_config::get_instance();

    unsigned int num_qubits = global_config.num_qubits;

    if (rs_addr >= (1 << RS_ADDR_WIDTH)) {
        return false;
    }

    if (rt_addr >= (1 << RT_ADDR_WIDTH)) {
        return false;
    }

    if (rd_addr >= (1 << RD_ADDR_WIDTH)) {
        return false;
    }

    if ((imm > ((1 << (LDI_IMM_WIDTH - 1)) - 1)) || (imm < (0 - (1 << (LDI_IMM_WIDTH - 1))))) {
        return false;
    }

    if (uimm > ((1 << LDUI_IMM_WIDTH) - 1)) {
        return false;
    }

    if ((br_addr > ((1 << (BR_ADDR_WIDTH - 1)) - 1)) ||
        (br_addr < (0 - (1 << (BR_ADDR_WIDTH - 1))))) {
        return false;
    }

    if (br_cond >= (1 << BR_COND_WIDTH)) {
        return false;
    }

    if (qubit_sel >= (1 << QUBIT_SEL_WIDTH)) {
        return false;
    }

    for (size_t i = 0; i < q_qubit_tuples.size(); ++i) {
        for (size_t j = 0; j < q_qubit_tuples[i].size(); ++j) {
            if (q_qubit_tuples[i][j] > num_qubits) {
                return false;
            }
        }
    }

    for (size_t i = 0; i < q_qubit_indices.size(); ++i) {
        if (q_qubit_indices[i] > num_qubits) {
            return false;
        }
    }

    for (size_t i = 0; i < q_reg_num.size(); ++i) {
        if (q_reg_num[i] >= 32) {
            return false;
        }
    }

    return true;
}

void Qasm_instruction::reset() {
    type                  = Instruction_type::BIN;
    insn_addr             = 0x0;
    insn_bin              = 0x0;
    insn_asm              = "";
    insn_line_num_in_file = "";
    insn_str_in_file      = "";
    opcode                = OperationName::NOP;
    rs_addr               = 0x1F;
    rt_addr               = 0x1F;
    rd_addr               = 0x1F;
    imm                   = 0x0;
    uimm                  = 0x0;
    br_addr               = 0x0;
    br_cond               = 0xF;
    qubit_sel             = 0x0;
    rd_used               = false;
    rs_used               = false;
    rt_used               = false;
    cl_insn               = false;
    q_insn                = false;
    meas_insn             = false;

    q_insn_type      = Q_instr_type::Q_NOP;
    q_time_specified = 0;

    q_qubit_indices.clear();
    q_qubit_tuples.clear();
    q_reg_num.clear();
    q_op_name.clear();
    q_num_tgt_qubits_type.clear();
}

void Qasm_instruction::set_instruction(const std::vector<std::string>&            insn,
                                       const std::map<std::string, unsigned int>& map_label,
                                       const unsigned int&                        addr) {
    auto logger = get_logger_or_exit("asm_logger");

    reset();

    type                  = Instruction_type::ASM;
    insn_line_num_in_file = insn[1];
    insn_str_in_file      = insn[2];
    insn_addr             = addr;
    insn_bin              = 0x0;
    insn_asm              = insn[0];

    parse_opcode(insn_asm);  // get opcode,cl_insn,q_insn,meas_insn

    if (cl_insn) {
        switch (opcode) {
            case OperationName::NOP:
                break;
            case OperationName::BR:
                parse_br(insn_asm, map_label, addr);
                break;
            case OperationName::CMP:
                parse_cmp(insn_asm);
                break;
            case OperationName::FBR:
                parse_fbr(insn_asm);
                break;
            case OperationName::FMR:
                parse_fmr(insn_asm);
                break;
            case OperationName::LDI:
                parse_ldi(insn_asm);
                break;
            case OperationName::LDUI:
                parse_ldui(insn_asm);
                break;
            case OperationName::SUB:
            case OperationName::ADD:
            case OperationName::REM:
            case OperationName::MUL:
            case OperationName::DIV:
                parse_arithmetic_operation(insn_asm);
                break;
            case OperationName::ADDI:
                parse_arithmetic_immediate_operation(insn_asm);
                break;
            case OperationName::AND:
            case OperationName::OR:
            case OperationName::XOR:
                parse_logic_operation(insn_asm);
                break;
            case OperationName::NOT:
                parse_not(insn_asm);
                break;
            case OperationName::LB:
            case OperationName::LBU:
            case OperationName::LW:
                parse_load_mem(insn_asm);
                break;
            case OperationName::SB:
            case OperationName::SW:
                parse_store_mem(insn_asm);
                break;
            case OperationName::STOP:
                break;
            default:
                logger->error(
                  "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
                  insn_str_in_file, insn_line_num_in_file);
                exit(EXIT_FAILURE);
        }
    }
    if (q_insn) {
        parse_qwaitr(insn_asm);
    }

    if (!verify_parser_result()) {
        logger->error(
          "asm_parser: Cannot parse asm instruction '{}' at line {}. Simulation aborts!",
          insn_str_in_file, insn_line_num_in_file);
        exit(EXIT_FAILURE);
    }
}

void Qasm_instruction::set_instruction(const unsigned int& insn, const unsigned int& addr) {
    auto logger = get_logger_or_exit("console");

    reset();

    type      = Instruction_type::BIN;
    insn_bin  = insn;
    insn_addr = addr;

    cl_insn = ((insn & 0xC0000000) == 0);  // !(insn[31] | insn[30])
    q_insn  = ((insn & 0xC0000000) != 0);  // (insn[31] | insn[30])

    bool meas_a = (insn & 0x80000000) & ((insn & 0x7E000000) == 1);
    bool meas_b = (insn & 0x80000000) & ((insn & 0x0001F800) == 1);
    meas_insn   = meas_a || meas_b;

    opcode = (insn & OPCODE_MASK) >> OPCODE_SHIFT;
    if (cl_insn) {
        switch (opcode) {
            case OperationName::NOP:
                break;
            case OperationName::BR:
                br_addr = (insn & BR_ADDR_MASK) >> BR_ADDR_SHIFT;
                br_cond = (insn & BR_COND_MASK) >> BR_COND_SHIFT;
                break;
            case OperationName::CMP:
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rs_used = true;
                rt_used = true;
                break;
            case OperationName::FBR:
                br_addr = (insn & BR_ADDR_MASK) >> BR_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                break;
            case OperationName::FMR:
                qubit_sel = (insn & QUBIT_SEL_MASK) >> QUBIT_SEL_SHIFT;
                rd_addr   = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used   = true;
                break;
            case OperationName::LDI:
                imm     = (insn & LDI_IMM_MASK) >> LDI_IMM_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                break;
            case OperationName::LDUI:
                uimm    = (insn & LDUI_IMM_MASK) >> LDUI_IMM_SHIFT;
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rs_used = true;
                break;
            case OperationName::SUB:
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rs_used = true;
                rt_used = true;
                break;
            case OperationName::ADD:
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rs_used = true;
                rt_used = true;
                break;
            case OperationName::AND:
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rs_used = true;
                rt_used = true;
                break;
            case OperationName::OR:
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rs_used = true;
                rt_used = true;
                break;
            case OperationName::XOR:
                rs_addr = (insn & RS_ADDR_MASK) >> RS_ADDR_SHIFT;
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rs_used = true;
                rt_used = true;
                break;
            case OperationName::NOT:
                rt_addr = (insn & RT_ADDR_MASK) >> RT_ADDR_SHIFT;
                rd_addr = (insn & RD_ADDR_MASK) >> RD_ADDR_SHIFT;
                rd_used = true;
                rt_used = true;
                break;
            case OperationName::STOP:
                break;
            default:
                logger->error(
                  "asm_parser: Cannot parse binary instruction '0x{:08x}' at line {}. Simulation "
                  "aborts!",
                  insn, addr);
                exit(EXIT_FAILURE);
        }
    }
}

Qasm_instruction& Qasm_instruction::operator=(const Qasm_instruction& insn) {
    if (this != &insn) {
        type                  = insn.type;
        insn_addr             = insn.insn_addr;
        insn_str_in_file      = insn.insn_str_in_file;
        insn_line_num_in_file = insn.insn_line_num_in_file;
        insn_bin              = insn.insn_bin;
        insn_asm              = insn.insn_asm;
        opcode                = insn.opcode;
        rs_addr               = insn.rs_addr;
        rt_addr               = insn.rt_addr;
        rd_addr               = insn.rd_addr;
        imm                   = insn.imm;
        uimm                  = insn.uimm;
        br_addr               = insn.br_addr;
        br_cond               = insn.br_cond;
        qubit_sel             = insn.qubit_sel;
        rd_used               = insn.rd_used;
        rs_used               = insn.rs_used;
        rt_used               = insn.rt_used;
        cl_insn               = insn.cl_insn;
        q_insn                = insn.q_insn;
        meas_insn             = insn.meas_insn;

        q_insn_type      = insn.q_insn_type;
        q_time_specified = insn.q_time_specified;

        q_qubit_indices.assign(insn.q_qubit_indices.begin(), insn.q_qubit_indices.end());
        q_qubit_tuples.assign(insn.q_qubit_tuples.begin(), insn.q_qubit_tuples.end());
        q_reg_num.assign(insn.q_reg_num.begin(), insn.q_reg_num.end());
        q_op_name.assign(insn.q_op_name.begin(), insn.q_op_name.end());
        q_num_tgt_qubits_type.assign(insn.q_num_tgt_qubits_type.begin(),
                                     insn.q_num_tgt_qubits_type.end());
    }
    return *this;
}

bool Qasm_instruction::operator==(const Qasm_instruction& insn) const {
    return (type == insn.type) && (insn_addr == insn.insn_addr) && (insn_bin == insn.insn_bin) &&
           (insn_asm == insn.insn_asm);
}

Qasm_instruction::Qasm_instruction() {
    reset();
    initial_map_opcode();
    initial_map_br_cond();
}

void Qasm_instruction::initial_map_opcode() {
    map_opcode.insert(std::make_pair("BR", OperationName::BR));
    map_opcode.insert(std::make_pair("br", OperationName::BR));
    map_opcode.insert(std::make_pair("CMP", OperationName::CMP));
    map_opcode.insert(std::make_pair("cmp", OperationName::CMP));
    map_opcode.insert(std::make_pair("FBR", OperationName::FBR));
    map_opcode.insert(std::make_pair("fbr", OperationName::FBR));
    map_opcode.insert(std::make_pair("FMR", OperationName::FMR));
    map_opcode.insert(std::make_pair("fmr", OperationName::FMR));
    map_opcode.insert(std::make_pair("LDI", OperationName::LDI));
    map_opcode.insert(std::make_pair("ldi", OperationName::LDI));
    map_opcode.insert(std::make_pair("LDUI", OperationName::LDUI));
    map_opcode.insert(std::make_pair("ldui", OperationName::LDUI));
    map_opcode.insert(std::make_pair("ADD", OperationName::ADD));
    map_opcode.insert(std::make_pair("add", OperationName::ADD));
    map_opcode.insert(std::make_pair("ADDI", OperationName::ADDI));
    map_opcode.insert(std::make_pair("addi", OperationName::ADDI));
    map_opcode.insert(std::make_pair("SUB", OperationName::SUB));
    map_opcode.insert(std::make_pair("sub", OperationName::SUB));
    map_opcode.insert(std::make_pair("AND", OperationName::AND));
    map_opcode.insert(std::make_pair("and", OperationName::AND));
    map_opcode.insert(std::make_pair("OR", OperationName::OR));
    map_opcode.insert(std::make_pair("or", OperationName::OR));
    map_opcode.insert(std::make_pair("XOR", OperationName::XOR));
    map_opcode.insert(std::make_pair("xor", OperationName::XOR));
    map_opcode.insert(std::make_pair("NOT", OperationName::NOT));
    map_opcode.insert(std::make_pair("not", OperationName::NOT));
    map_opcode.insert(std::make_pair("LB", OperationName::LB));
    map_opcode.insert(std::make_pair("lb", OperationName::LB));
    map_opcode.insert(std::make_pair("LBU", OperationName::LBU));
    map_opcode.insert(std::make_pair("lbu", OperationName::LBU));
    map_opcode.insert(std::make_pair("LW", OperationName::LW));
    map_opcode.insert(std::make_pair("lw", OperationName::LW));
    map_opcode.insert(std::make_pair("SB", OperationName::SB));
    map_opcode.insert(std::make_pair("sb", OperationName::SB));
    map_opcode.insert(std::make_pair("SW", OperationName::SW));
    map_opcode.insert(std::make_pair("sw", OperationName::SW));
    map_opcode.insert(std::make_pair("REM", OperationName::REM));
    map_opcode.insert(std::make_pair("rem", OperationName::REM));
    map_opcode.insert(std::make_pair("MUL", OperationName::MUL));
    map_opcode.insert(std::make_pair("mul", OperationName::MUL));
    map_opcode.insert(std::make_pair("DIV", OperationName::DIV));
    map_opcode.insert(std::make_pair("div", OperationName::DIV));
    map_opcode.insert(std::make_pair("NOP", OperationName::NOP));
    map_opcode.insert(std::make_pair("nop", OperationName::NOP));
    map_opcode.insert(std::make_pair("STOP", OperationName::STOP));
    map_opcode.insert(std::make_pair("stop", OperationName::STOP));
}

void Qasm_instruction::initial_map_br_cond() {
    map_br_cond.insert(std::make_pair("ALWAYS", 0));
    map_br_cond.insert(std::make_pair("always", 0));
    map_br_cond.insert(std::make_pair("NEVER", 1));
    map_br_cond.insert(std::make_pair("never", 1));
    map_br_cond.insert(std::make_pair("EQ", 2));
    map_br_cond.insert(std::make_pair("eq", 2));
    map_br_cond.insert(std::make_pair("NE", 3));
    map_br_cond.insert(std::make_pair("ne", 3));
    map_br_cond.insert(std::make_pair("LTU", 8));
    map_br_cond.insert(std::make_pair("ltu", 8));
    map_br_cond.insert(std::make_pair("GEU", 9));
    map_br_cond.insert(std::make_pair("geu", 9));
    map_br_cond.insert(std::make_pair("LEU", 10));
    map_br_cond.insert(std::make_pair("leu", 10));
    map_br_cond.insert(std::make_pair("GTU", 11));
    map_br_cond.insert(std::make_pair("gtu", 11));
    map_br_cond.insert(std::make_pair("LT", 12));
    map_br_cond.insert(std::make_pair("lt", 12));
    map_br_cond.insert(std::make_pair("GE", 13));
    map_br_cond.insert(std::make_pair("ge", 13));
    map_br_cond.insert(std::make_pair("LE", 14));
    map_br_cond.insert(std::make_pair("le", 14));
    map_br_cond.insert(std::make_pair("GT", 15));
    map_br_cond.insert(std::make_pair("gt", 15));
}

Qasm_instruction::~Qasm_instruction() {}

}  // namespace cactus
