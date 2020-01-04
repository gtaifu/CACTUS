#ifndef _Q_DATA_TYPE_H_
#define _Q_DATA_TYPE_H_

#include <cassert>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <systemc>

#include "global_json.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_vector;
using sc_dt::sc_uint;
using std::string;
using std::to_string;

// --------------------------------------------------------------------------------------------
// Classical instruction-related constants.
// --------------------------------------------------------------------------------------------

#define OPCODE_SHIFT 25
#define OPCODE_MASK (0x3F << OPCODE_SHIFT)

#define RD_ADDR_SHIFT 20
#define RD_ADDR_WIDTH 5
#define RD_ADDR_MASK (0x1F << RD_ADDR_SHIFT)

#define RS_ADDR_SHIFT 15
#define RS_ADDR_WIDTH 5
#define RS_ADDR_MASK (0x1F << RS_ADDR_SHIFT)

#define RT_ADDR_SHIFT 10
#define RT_ADDR_WIDTH 5
#define RT_ADDR_MASK (0x1F << RT_ADDR_SHIFT)

#define BR_COND_SHIFT 0
#define BR_COND_WIDTH 4
#define BR_COND_MASK (0xF << BR_COND_SHIFT)

#define BR_ADDR_SHIFT 4
#define BR_ADDR_WIDTH 21
#define BR_ADDR_MASK (0x1FFFFF << BR_ADDR_SHIFT)

#define COMP_FLAG_SHIFT 0
#define COMP_FLAG_WIDTH 4
#define COMP_FLAG_MASK (0xF << COMP_FLAG_SHIFT)

#define QUBIT_SEL_SHIFT 0
#define QUBIT_SEL_WIDTH 3
#define QUBIT_SEL_MASK (0x7 << QUBIT_SEL_SHIFT)

#define LD_IMM_SHIFT 0
#define LD_IMM_WIDTH 10
#define LD_IMM_MASK (0x3FF << LD_IMM_SHIFT)

#define LDI_IMM_SHIFT 0
#define LDI_IMM_WIDTH 20
#define LDI_IMM_MASK (0xFFFFF << LDI_IMM_SHIFT)

#define LDUI_IMM_SHIFT 0
#define LDUI_IMM_WIDTH 15
#define LDUI_IMM_MASK (0x7FFF << LDUI_IMM_SHIFT)

enum OperationName {
    NOP  = 0,
    BR   = 1,
    ADDI = 2,
    STOP = 8,
    LW   = 9,
    SW   = 10,
    TEST = 12,
    CMP  = 13,
    LB   = 14,
    LBU  = 15,
    SB   = 16,
    DIV  = 17,
    MUL  = 18,
    REM  = 19,
    FBR  = 20,
    FMR  = 21,
    LDI  = 22,
    LDUI = 23,
    OR   = 24,
    XOR  = 25,
    AND  = 26,
    NOT  = 27,
    ADDC = 28,
    SUBC = 29,
    ADD  = 30,
    SUB  = 31
};

enum Q_instr_type {
    Q_NOP   = 0,
    Q_STOP  = 1,
    Q_SMIS  = 2,
    Q_SMIT  = 3,
    Q_WAIT  = 4,
    Q_WAITR = 5,
    Q_OP    = 6
};

typedef size_t qubit_id_t;

// Conditions flags. See eQASM reference manual for the definition.
enum Conditions { NEVER = 0, ALWAYS, EQ, NE, LTU, GEU, LEU, GTU, LT, GE, LE, GT };

// load and store type
enum MEM_ACCESS_TYPE { ADDR_BYTE = 0, ADDR_HALF_WORD, ADDR_WORD };

// --------------------------------------------------------------------------------------------
// QISA format definition
// --------------------------------------------------------------------------------------------
class Multi_Q_insn {
  public:
    // TODO. check if the width can be loaded from configuration file.
    sc_uint<M_OPCODE_WIDTH> opcode;
    sc_uint<REG_ADDR_WIDTH> mask_reg_addr;

    Multi_Q_insn(unsigned int op = 0, unsigned int m_reg_addr = 0) {
        this->opcode        = op;
        this->mask_reg_addr = m_reg_addr;
    };

    ~Multi_Q_insn(){};

    int get_width() { return (M_OPCODE_WIDTH + REG_ADDR_WIDTH); };
};

bool operator==(const Multi_Q_insn& a, const Multi_Q_insn& b);

string to_string(const Multi_Q_insn& u_insn);

std::ostream& operator<<(std::ostream& os, const Multi_Q_insn& u_insn);

void sc_trace(sc_core::sc_trace_file* tf, const Multi_Q_insn& u_insn, const std::string& name);

// --------------------------------------------------------------------------------------------
// timing point
// --------------------------------------------------------------------------------------------
class Timing_point {
  public:
    Timing_point() {
        this->pre_interval = 0;
        this->label        = 0;
    };

    sc_uint<WAIT_TIME_WIDTH>   pre_interval;
    sc_uint<MAJOR_LABEL_WIDTH> label;
};

bool operator==(const Timing_point& a, const Timing_point& b);

string to_string(const Timing_point& tp);

std::ostream& operator<<(std::ostream& os, const Timing_point& tp);

void sc_trace(sc_core::sc_trace_file* tf, const Timing_point& tp, const std::string& name);

// Single-qubit gate or two-qubit gate
// 0: single-qubit gate; 1: two-qubit gate
typedef sc_dt::sc_uint<1> S_OR_T;

enum OP_TYPE_DEVICE : unsigned int {
    QNOP        = 0,
    MICROWAVE   = 1,
    FLUX        = 2,
    MEASUREMENT = 3,
};
// typedef sc_core::sc_uint<OPERATION_TYPE_WIDTH> OP_TYPE_DEVICE;

// --------------------------------------------------------------------------------------------
// Operation format
// --------------------------------------------------------------------------------------------
class Operation {
  public:
    sc_uint<OPERATION_TYPE_WIDTH> type;
    sc_uint<MAX_CODEWORD_WIDTH>   codeword;

  public:
    Operation(sc_uint<OPERATION_TYPE_WIDTH> op_type = 0, sc_uint<MAX_CODEWORD_WIDTH> cw = 0);

    Operation operator|(const Operation& other);

    bool is_nop();

    void reset();
};

bool operator==(const Operation& a, const Operation& b);

bool operator!=(const Operation& a, const Operation& b);

string to_string(const Operation& e);

std::ostream& operator<<(std::ostream& os, const Operation& e);

void sc_trace(sc_core::sc_trace_file* tf, const Operation& e, const std::string& name);

// --------------------------------------------------------------------------------------------
// control store line format definition
// --------------------------------------------------------------------------------------------
class CS_line {
  public:
    Operation                  op_left;
    Operation                  op_right;
    sc_uint<COND_WIDTH>        condbits;
    sc_uint<MINOR_LABEL_WIDTH> minor_label;
    bool                       u_insn_eoi;  // End of QISA indicator

    CS_line()
        : op_left()
        , op_right() {
        minor_label = 0;
        u_insn_eoi  = false;
    }
};

bool operator==(const CS_line& a, const CS_line& b);

string to_string(const CS_line& line);

std::ostream& operator<<(std::ostream& os, const CS_line& line);

void sc_trace(sc_core::sc_trace_file* tf, const CS_line& line, const std::string& name);

// --------------------------------------------------------------------------------------------
// MUX type  definition
// which is used in the address resolver
// --------------------------------------------------------------------------------------------
extern std::string op_sel_name[];

class Op_sel {
  public:
    // 0b00 : no operation;       0b10 : right operation
    // 0b01 : left operation;     0b11 : single-qubit gate (left)
    sc_uint<2> val;

    Op_sel() { this->val = 0; };
    Op_sel(const Op_sel& other) { this->val = other.val; };

    Op_sel operator=(const Op_sel& other);
};

bool operator==(const Op_sel& a, const Op_sel& b);

bool operator!=(const Op_sel& a, const Op_sel& b);

string to_string(const Op_sel& sel);

std::ostream& operator<<(std::ostream& os, const Op_sel& sel);

void sc_trace(sc_core::sc_trace_file* tf, const Op_sel& sel, const std::string& name);

// --------------------------------------------------------------------------------------------
// microinstruction
// --------------------------------------------------------------------------------------------
// class Microinstruction {
// public:
//     sc_vector<Op_sel>                 vec_op_sel;
//     Operation                         op_left;
//     Operation                         op_right;
//     sc_uint<MAJOR_LABEL_WIDTH>        major_label;
//     sc_uint<MINOR_LABEL_WIDTH>        minor_label;

// protected:
//     int     num_qubits;

//     void config(Global_config& global_config) {
//         num_qubits          = global_config.num_qubits;
//     };

// public:
//     Microinstruction(Global_config& global_config);

//     bool operator==(const Microinstruction &other);
// };

// string to_string(const Microinstruction &u_insn);

// std::ostream &operator<<(std::ostream &os, const Microinstruction &u_insn);

// void sc_trace(sc_core::sc_trace_file *tf, const Microinstruction &u_insn,
//                      const std::string &name);

// --------------------------------------------------------------------------------------------
// Micro operation
// --------------------------------------------------------------------------------------------
class Micro_operation {
  public:
    Operation                  u_operation;
    sc_uint<COND_WIDTH>        condbits;
    sc_uint<MAJOR_LABEL_WIDTH> major_label;
    sc_uint<MINOR_LABEL_WIDTH> minor_label;

  public:
    Micro_operation();

    Micro_operation(sc_uint<2> type, sc_uint<MAX_CODEWORD_WIDTH> cw, sc_uint<COND_WIDTH> cond,
                    sc_uint<MAJOR_LABEL_WIDTH> ma_l, sc_uint<MAJOR_LABEL_WIDTH> mi_l);

    Micro_operation(const Operation& op, sc_uint<COND_WIDTH> cond, sc_uint<MAJOR_LABEL_WIDTH> ma_l,
                    sc_uint<MAJOR_LABEL_WIDTH> mi_l);

    // void reset(const Micro_operation &u_op);

    Micro_operation operator|(const Micro_operation& other);

    Micro_operation operator|=(const Micro_operation& other);

    bool is_nop();

    void reset();
};

bool operator==(const Micro_operation& a, const Micro_operation& b);

bool operator!=(const Micro_operation& a, const Micro_operation& b);

string to_string(const Micro_operation& u_op);

std::ostream& operator<<(std::ostream& os, const Micro_operation& u_op);

void sc_trace(sc_core::sc_trace_file* tf, const Micro_operation& u_op, const std::string& name);

// --------------------------------------------------------------------------------------------
// Micro operation for qubit
// --------------------------------------------------------------------------------------------
class Micro_operation_for_qubit {
  public:
    Operation           u_operation;
    sc_uint<COND_WIDTH> condbits;

  public:
    Micro_operation_for_qubit();

    Micro_operation_for_qubit(sc_uint<2> type, sc_uint<MAX_CODEWORD_WIDTH> cw,
                              sc_uint<COND_WIDTH> cond);

    Micro_operation_for_qubit(const Operation& op, sc_uint<COND_WIDTH> cond);

    Micro_operation_for_qubit operator|=(const Micro_operation_for_qubit& other);
};

bool operator==(const Micro_operation_for_qubit& a, const Micro_operation_for_qubit& b);

/*void align (const Micro_operation &u_op1, const Micro_operation &u_op2) {
return Micro_operation(u_op1.u_operation | u_op2.u_operation,)
}*/

string to_string(const Micro_operation_for_qubit& u_op);

std::ostream& operator<<(std::ostream& os, const Micro_operation_for_qubit& u_op);

void sc_trace(sc_core::sc_trace_file* tf, const Micro_operation_for_qubit& u_op,
              const std::string& name);

// --------------------------------------------------------------------------------------------
// General device event, which is the base class for all kinds of device events
// --------------------------------------------------------------------------------------------
class Device_event_base {
  public:
    sc_uint<MAJOR_LABEL_WIDTH> major_label;
    sc_uint<MINOR_LABEL_WIDTH> minor_label;

  public:
    Device_event_base();

    void reset() {
        major_label = 0;
        minor_label = 0;
    }
};

bool operator==(const Device_event_base& a, const Device_event_base& b);

string to_string(const Device_event_base& event);

std::ostream& operator<<(std::ostream& os, const Device_event_base& event);

void sc_trace(sc_core::sc_trace_file* tf, const Device_event_base& event, const std::string& name);

// --------------------------------------------------------------------------------------------
// Microwave DIO event
// --------------------------------------------------------------------------------------------
class MW_DIO_event : public Device_event_base {
  public:
    sc_uint<DIO_WIDTH> codeword;

  public:
    MW_DIO_event();

    void reset() {
        Device_event_base::reset();
        codeword = 0;
    }
};

bool operator==(const MW_DIO_event& a, const MW_DIO_event& b);

string to_string(const MW_DIO_event& event);

std::ostream& operator<<(std::ostream& os, const MW_DIO_event& event);

void sc_trace(sc_core::sc_trace_file* tf, const MW_DIO_event& event, const std::string& name);

// --------------------------------------------------------------------------------------------
// VSM event
// --------------------------------------------------------------------------------------------
class VSM_event : public Device_event_base {
  public:
    sc_uint<64>              mask;
    sc_uint<MSMT_COND_WIDTH> condbits;
    // sc_uint<64>							mask_value;

  public:
    VSM_event();
};

bool operator==(const VSM_event& a, const VSM_event& b);

string to_string(const VSM_event& event);

std::ostream& operator<<(std::ostream& os, const VSM_event& event);

void sc_trace(sc_core::sc_trace_file* tf, const VSM_event& event, const std::string& name);

// --------------------------------------------------------------------------------------------
// Measurement DIO event
// --------------------------------------------------------------------------------------------
class MSMT_DIO_event : public Device_event_base {
  public:
    sc_uint<MSMT_COND_WIDTH>     condbits;
    sc_uint<NUM_QUBITS_FEEDLINE> valid;

  public:
    MSMT_DIO_event();
};

bool operator==(const MSMT_DIO_event& a, const MSMT_DIO_event& b);

string to_string(const MSMT_DIO_event& event);

std::ostream& operator<<(std::ostream& os, const MSMT_DIO_event& event);

void sc_trace(sc_core::sc_trace_file* tf, const MSMT_DIO_event& event, const std::string& name);

// --------------------------------------------------------------------------------------------
// Flux DIO event
// --------------------------------------------------------------------------------------------
class Flux_DIO_event : public Device_event_base {
  public:
    sc_uint<FLUX_COND_WIDTH> condbits;
    sc_uint<FLUX_CW_WIDTH>   codeword;

  public:
    Flux_DIO_event();
};

bool operator==(const Flux_DIO_event& a, const Flux_DIO_event& b);

string to_string(const Flux_DIO_event& event);

std::ostream& operator<<(std::ostream& os, const Flux_DIO_event& event);

void sc_trace(sc_core::sc_trace_file* tf, const Flux_DIO_event& event, const std::string& name);

};  // namespace cactus

#endif  // _Q_DATA_TYPE_H_
