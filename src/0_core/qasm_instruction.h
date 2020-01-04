/** qasm_instruction.h
 *
 * This file defines the qasm instruction object.
 *
 * It supports binary and assemble format.
 *
 */

#ifndef _QASM_INSTRUCTION_H_
#define _QASM_INSTRUCTION_H_

#include <map>
#include <string>
#include <systemc>

#include "generic_if.h"
#include "q_data_type.h"

namespace cactus {

class Qasm_instruction {
  public:
    Instruction_type type;
    unsigned int     insn_addr;
    unsigned int     insn_bin;
    std::string      insn_asm;
    std::string      insn_line_num_in_file;
    std::string      insn_str_in_file;

  private:  // member variables
    unsigned int                        opcode;
    std::map<std::string, unsigned int> map_opcode;
    unsigned int                        rs_addr;
    unsigned int                        rd_addr;
    unsigned int                        rt_addr;
    int                                 imm;
    unsigned                            uimm;
    // br_cond is used for br and fbr
    int                                 br_addr;
    unsigned int                        br_cond;
    std::map<std::string, unsigned int> map_br_cond;
    unsigned int                        qubit_sel;
    Q_instr_type                        q_insn_type;
    std::vector<size_t>                 q_qubit_indices;
    std::vector<std::vector<size_t>>    q_qubit_tuples;
    std::vector<unsigned int>           q_reg_num;
    std::vector<std::string>            q_op_name;
    std::vector<num_tgt_qubits_type_t>  q_num_tgt_qubits_type;
    unsigned int                        q_time_specified;

    bool rd_used;    // is rd used in this instruction
    bool rs_used;    // is rs used in this instruction
    bool rt_used;    // is rt used in this instruction
    bool cl_insn;    // classical instruction
    bool q_insn;     // quantum instruction
    bool meas_insn;  // measure instruction

    unsigned int m_num_qubits;

  public:  // member function
    Instruction_type                    get_type();
    unsigned int                        get_insn_bin();
    std::string                         get_insn_asm();
    std::string                         get_insn_line_num_in_file();
    std::string                         get_insn_str_in_file();
    unsigned int                        get_opcode();
    unsigned int                        get_rs_addr();
    unsigned int                        get_rt_addr();
    unsigned int                        get_rd_addr();
    unsigned int                        get_uimm();
    int                                 get_imm();
    int                                 get_br_addr();
    unsigned int                        get_br_cond();
    unsigned int                        get_qubit_sel();
    Q_instr_type                        get_q_insn_type();
    unsigned int                        get_q_time_specified();
    std::vector<size_t>&                get_q_qubit_indices();
    std::vector<std::vector<size_t>>&   get_q_qubit_tuples();
    std::vector<unsigned int>&          get_q_reg_num();
    std::vector<std::string>&           get_q_op_name();
    std::vector<num_tgt_qubits_type_t>& get_q_num_tgt_qubits_type();

    bool is_q_insn();
    bool is_rd_used();
    bool is_rs_used();
    bool is_rt_used();
    bool is_cl_insn();
    bool is_meas();
    bool is_fmr();
    bool is_br();
    bool is_stop();

    // NOP and STOP do not need parse
    void parse_opcode(const std::string& insn);
    void parse_br(const std::string& insn, const std::map<std::string, unsigned int>& map_label,
                  const unsigned int& addr);
    void parse_cmp(const std::string& insn);
    void parse_fbr(const std::string& insn);
    void parse_fmr(const std::string& insn);
    void parse_ldi(const std::string& insn);
    void parse_ldui(const std::string& insn);
    void parse_not(const std::string& insn);
    void parse_qwaitr(const std::string& insn);

    void parse_load_mem(const std::string& insn);
    void parse_store_mem(const std::string& insn);

    void parse_logic_operation(const std::string& insn);
    void parse_arithmetic_operation(const std::string& insn);
    void parse_arithmetic_immediate_operation(const std::string& insn);

    void parse_q_insn();
    void parse_q_insn_type();
    void parse_qwait();
    void parse_smis();
    void parse_smit();
    void verify_rotate_angle(const std::string op_name, std::string& op_name_prefix);
    void parse_qop();

    unsigned int verify_parser_result();

  public:
    void reset();

    void set_instruction(const std::vector<std::string>&            insn,
                         const std::map<std::string, unsigned int>& map_label,
                         const unsigned int&                        addr);

    void set_instruction(const unsigned int& insn, const unsigned int& addr);

    Qasm_instruction& operator=(const Qasm_instruction& insn);  // operation =

    bool operator==(const Qasm_instruction& insn) const;  // operator ==

    void initial_map_opcode();
    void initial_map_br_cond();

  public:
    Qasm_instruction();

    ~Qasm_instruction();
};

inline void sc_trace(sc_core::sc_trace_file* tf, const Qasm_instruction& insn,
                     const std::string& name) {}

inline std::ostream& operator<<(std::ostream& os, const Qasm_instruction& insn) { return os; }

}  // namespace cactus
#endif  //_QASM_INSTRUCTION_H_