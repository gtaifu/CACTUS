#include "q_data_type.h"

#include <iomanip>
#include <sstream>

#include "generic_if.h"

namespace cactus {
bool operator==(const Multi_Q_insn& a, const Multi_Q_insn& b) {
    return ((a.opcode == b.opcode) && (a.mask_reg_addr == b.mask_reg_addr));
}

string to_string(const Multi_Q_insn& u_insn) {
    string _str_ =
      "(op: " + to_string(u_insn.opcode) + " mask: " + to_string(u_insn.mask_reg_addr) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Multi_Q_insn& u_insn) {
    return os << std::setw(20) << to_string(u_insn);
}

void sc_trace(sc_core::sc_trace_file* tf, const Multi_Q_insn& u_insn, const std::string& name) {
    sc_trace(tf, u_insn.opcode, name + ".op");
    sc_trace(tf, u_insn.mask_reg_addr, name + ".mask_reg_addr");
}

// --------------------------------------------------------------------------------------------
// timing point
// --------------------------------------------------------------------------------------------
bool operator==(const Timing_point& a, const Timing_point& b) {
    return ((a.pre_interval == b.pre_interval) && (a.label == b.label));
}

string to_string(const Timing_point& tp) {
    string _str_ =
      "TP(intvl: " + to_string(tp.pre_interval) + ", label: " + to_string(tp.label) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Timing_point& tp) { return os << to_string(tp); }

void sc_trace(sc_core::sc_trace_file* tf, const Timing_point& tp, const std::string& name) {
    sc_trace(tf, tp.label, name + ".label");
    sc_trace(tf, tp.pre_interval, name + ".pre_interval");
}

// --------------------------------------------------------------------------------------------
// Operation format
// --------------------------------------------------------------------------------------------
Operation::Operation(sc_uint<OPERATION_TYPE_WIDTH> op_type, sc_uint<MAX_CODEWORD_WIDTH> cw) {
    type     = (unsigned int) (op_type);
    codeword = cw;
}

bool operator==(const Operation& a, const Operation& b) {
    return ((a.type == b.type) && (a.codeword == b.codeword));
}

bool operator!=(const Operation& a, const Operation& b) { return not(a == b); }

Operation Operation::operator|(const Operation& other) {
    return Operation((this->type | other.type), (this->codeword | other.codeword));
}

bool Operation::is_nop() { return (type == 0); }

void Operation::reset() {
    type     = 0;
    codeword = 0;
}

string to_string(const Operation& e) {
    std::stringstream ss;
    ss << "Op(T: " << int(e.type) << ", CW: " << std::setw(3) << std::setfill(' ')
       << int(e.codeword) << ")";
    return ss.str();
    // string _str_ =
    //   "Op(T: " + to_string(int(e.type)) + ", CW: " + to_string(e.codeword) + ")";
    // return _str_;
}

std::ostream& operator<<(std::ostream& os, const Operation& e) { return os << to_string(e); }

void sc_trace(sc_core::sc_trace_file* tf, const Operation& e, const std::string& name) {
    sc_trace(tf, e.type, name + ".type");
    sc_trace(tf, e.codeword, name + ".codeword");
}

// --------------------------------------------------------------------------------------------
// control store line format definition
// --------------------------------------------------------------------------------------------
bool operator==(const CS_line& a, const CS_line& b) {
    return ((a.op_left == b.op_left) && (a.op_right == b.op_right) && (a.condbits == b.condbits) &&
            (a.minor_label == b.minor_label) && (a.u_insn_eoi == b.u_insn_eoi));
}

string to_string(const CS_line& line) {
    string _str_ = "CS_Line[ " + to_string(line.op_left) + ", " + to_string(line.op_right) +
                   "  cond:" + to_string(int(line.condbits)) +
                   "  minor_label: " + to_string(line.minor_label) +
                   "  eoi: " + to_string(line.u_insn_eoi) + "]";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const CS_line& line) { return os << to_string(line); }

void sc_trace(sc_core::sc_trace_file* tf, const CS_line& line, const std::string& name) {
    sc_trace(tf, line.op_left, name + ".op_left");
    sc_trace(tf, line.op_right, name + ".op_right");
    sc_trace(tf, line.condbits, name + ".condbits");
    sc_trace(tf, line.minor_label, name + ".minor_label");
    sc_trace(tf, line.u_insn_eoi, name + ".eoi");
}

// --------------------------------------------------------------------------------------------
// MUX type  definition
// --------------------------------------------------------------------------------------------
std::string op_sel_name[] = { "nop", "left", "right", "single" };
// op_sel_name[1] = "nop";
// op_sel_name[2] = "left";
// op_sel_name[3] = "right";
// op_sel_name[4] = "single";

bool operator==(const Op_sel& a, const Op_sel& b) { return (a.val == b.val); }

bool operator!=(const Op_sel& a, const Op_sel& b) { return (a.val != b.val); }

Op_sel Op_sel::operator=(const Op_sel& other) {
    this->val = other.val;
    return *this;
}

string to_string(const Op_sel& sel) {
    string _str_ = "[mux: ";
    switch (sel.val) {
        case 0:
            _str_ += "nop";
            break;
        case 1:
            _str_ += "left";
            break;
        case 2:
            _str_ += "right";
            break;
        case 3:
            _str_ += "single";
            break;
        default:
            assert(false);
    }
    _str_ += "]";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Op_sel& sel) { return os << to_string(sel); }

void sc_trace(sc_core::sc_trace_file* tf, const Op_sel& sel, const std::string& name) {
    sc_trace(tf, sel.val, name + ".mux_type");
}

// --------------------------------------------------------------------------------------------
// microinstruction
// --------------------------------------------------------------------------------------------
// Microinstruction::Microinstruction(Global_config& global_config) : op_left(), op_right() {
//     config(global_config);

//     vec_op_sel.init(num_qubits);

//     major_label = 0;
//     minor_label = 0;
// }

// bool Microinstruction::operator==(const Microinstruction &other) {
//     bool addr_equal = true;
//     for (size_t i = 0; i < this->vec_op_sel.size(); ++i)
//     {
//         if (this->vec_op_sel[i] != other.vec_op_sel[i])
//         {
//             addr_equal = false;
//             break;
//         }
//     }

//     return ( addr_equal &&
//             (this->op_left == other.op_left) &&
//             (this->op_right == other.op_right) &&
//             (this->major_label == other.major_label) &&
//             (this->minor_label == other.minor_label) );
// }

// string to_string(const Microinstruction &u_insn) {
//     string _str_ = "(u_insn: {op_sels:";
//     for (size_t i = 0; i < u_insn.vec_op_sel.size(); ++i)
//     {
//         _str_ += (" " + to_string(u_insn.vec_op_sel[i]));
//     }
//     _str_ += "} left_op: " + to_string(u_insn.op_left) + \
    //             ", right_op: " + to_string(u_insn.op_right) + \
    //             ", major_label: " + to_string(u_insn.major_label) + \
    //             ", minor_label: " + to_string(u_insn.minor_label) + ")";
//     return _str_;
// };

// std::ostream &operator<<(std::ostream &os, const Microinstruction &u_insn) {
//     return os << to_string(u_insn);
// };

// void sc_trace(sc_core::sc_trace_file *tf, const Microinstruction &u_insn, const std::string
// &name) {
//     for (size_t i = 0; i < u_insn.vec_op_sel.size(); ++i)
//     {
//         sc_trace(tf, u_insn.vec_op_sel[i], name + ".op_sel"+to_string(i));
//     }
//     sc_trace(tf, u_insn.op_left, name + ".op_left");
//     sc_trace(tf, u_insn.op_right, name + ".op_right");
//     sc_trace(tf, u_insn.major_label, name + ".major_label");
//     sc_trace(tf, u_insn.minor_label, name + ".minor_label");
// };

// --------------------------------------------------------------------------------------------
// Micro operation
// --------------------------------------------------------------------------------------------
Micro_operation::Micro_operation()
    : u_operation(0, 0) {
    condbits    = 0;
    major_label = 0;
    minor_label = 0;
}

Micro_operation::Micro_operation(sc_uint<2> type, sc_uint<MAX_CODEWORD_WIDTH> cw,
                                 sc_uint<COND_WIDTH> cond, sc_uint<MAJOR_LABEL_WIDTH> ma_l,
                                 sc_uint<MAJOR_LABEL_WIDTH> mi_l)
    : u_operation(type, cw) {
    condbits    = cond;
    major_label = ma_l;
    minor_label = mi_l;
}

Micro_operation::Micro_operation(const Operation& op, sc_uint<COND_WIDTH> cond,
                                 sc_uint<MAJOR_LABEL_WIDTH> ma_l, sc_uint<MAJOR_LABEL_WIDTH> mi_l)
    : u_operation(op) {
    condbits    = cond;
    major_label = ma_l;
    minor_label = mi_l;
}

bool operator==(const Micro_operation& a, const Micro_operation& b) {
    return ((a.u_operation == b.u_operation) && (a.condbits == b.condbits) &&
            (a.major_label == b.major_label) && (a.minor_label == b.minor_label));
}

bool operator!=(const Micro_operation& a, const Micro_operation& b) { return not(a == b); }

/*void Micro_operation::reset(const Micro_operation &u_op) {
    u_op.u_operation.codeword = 0;
    u_op.u_operation.type = 0;
    u_op.condbits = 0;
}*/

Micro_operation Micro_operation::operator|(const Micro_operation& other) {
    return Micro_operation(this->u_operation | other.u_operation, this->condbits | other.condbits,
                           this->major_label | other.major_label,
                           this->minor_label | other.minor_label);
}

Micro_operation Micro_operation::operator|=(const Micro_operation& other) {
    this->u_operation = this->u_operation | other.u_operation;
    this->condbits    = this->condbits | other.condbits;
    this->major_label |= other.major_label;
    this->minor_label |= other.minor_label;
    return *this;
}

bool Micro_operation::is_nop() { return this->u_operation.is_nop(); }

void Micro_operation::reset() {
    this->u_operation.reset();
    this->condbits    = 0;
    this->major_label = 0;
    this->minor_label = 0;
}

string to_string(const Micro_operation& u_op) {
    // string _str_ =  "u_op(" + to_string(u_op.u_operation) +\
        //                 ", condbits: " + to_string(int(u_op.condbits)) +\
        //                 ", major_label: " + to_string(u_op.major_label) +\
        //                 ", minor_label: " + to_string(u_op.minor_label) + ")";
    string _str_ = "(t: " + to_string(int(u_op.u_operation.type)) +
                   " cw: " + to_string(u_op.u_operation.codeword) +
                   " c: " + to_string(int(u_op.condbits)) + " l: " + to_string(u_op.major_label) +
                   ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Micro_operation& u_op) {
    return os << to_string(u_op);
}

void sc_trace(sc_core::sc_trace_file* tf, const Micro_operation& u_op, const std::string& name) {
    sc_trace(tf, u_op.u_operation, name + ".u_op");
    sc_trace(tf, u_op.condbits, name + ".condbits");
    sc_trace(tf, u_op.major_label, name + ".major_label");
    sc_trace(tf, u_op.minor_label, name + ".minor_label");
}

// --------------------------------------------------------------------------------------------
// Micro operation for qubit
// --------------------------------------------------------------------------------------------
Micro_operation_for_qubit::Micro_operation_for_qubit()
    : u_operation(0, 0) {
    condbits = 0;
}

Micro_operation_for_qubit::Micro_operation_for_qubit(sc_uint<2>                  type,
                                                     sc_uint<MAX_CODEWORD_WIDTH> cw,
                                                     sc_uint<COND_WIDTH>         cond)
    : u_operation(type, cw) {
    condbits = cond;
}

Micro_operation_for_qubit::Micro_operation_for_qubit(const Operation& op, sc_uint<COND_WIDTH> cond)
    : u_operation(op) {
    condbits = cond;
}

bool operator==(const Micro_operation_for_qubit& a, const Micro_operation_for_qubit& b) {
    return ((a.u_operation == b.u_operation) && (a.condbits == b.condbits));
}

Micro_operation_for_qubit Micro_operation_for_qubit::operator|=(
  const Micro_operation_for_qubit& other) {
    this->u_operation = this->u_operation | other.u_operation;
    this->condbits    = this->condbits | other.condbits;
    return *this;
}

string to_string(const Micro_operation_for_qubit& u_op) {
    string _str_ =
      "u_op(" + to_string(u_op.u_operation) + ", condbits: " + to_string(int(u_op.condbits)) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Micro_operation_for_qubit& u_op) {
    return os << to_string(u_op);
}

void sc_trace(sc_core::sc_trace_file* tf, const Micro_operation_for_qubit& u_op,
              const std::string& name) {
    sc_trace(tf, u_op.u_operation, name + ".u_op");
    sc_trace(tf, u_op.condbits, name + ".condbits");
}

// --------------------------------------------------------------------------------------------
// General device event, which is the base class for all kinds of device events
// --------------------------------------------------------------------------------------------
Device_event_base::Device_event_base() {
    major_label = 0;
    minor_label = 0;
}

bool operator==(const Device_event_base& a, const Device_event_base& b) {
    return ((a.major_label == b.major_label) && (a.minor_label == b.minor_label));
}

string to_string(const Device_event_base& event) {
    string _str_ = "major_label: " + to_string(event.major_label) +
                   ", minor_label: " + to_string(event.minor_label);
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Device_event_base& event) {
    return os << to_string(event);
}

void sc_trace(sc_core::sc_trace_file* tf, const Device_event_base& event, const std::string& name) {
    sc_trace(tf, event.major_label, name + ".major_label");
    sc_trace(tf, event.minor_label, name + ".minor_label");
}

// --------------------------------------------------------------------------------------------
// Microwave DIO event
// --------------------------------------------------------------------------------------------
MW_DIO_event::MW_DIO_event()
    : Device_event_base() {
    codeword = 0;
}

bool operator==(const MW_DIO_event& a, const MW_DIO_event& b) {
    return ((a.codeword == b.codeword) && (a.major_label == b.major_label) &&
            (a.minor_label == b.minor_label));
}

string to_string(const MW_DIO_event& event) {
    string _str_ = "(Trig:" + to_string(event.codeword[31]) +
                   ", CW: " + to_string(event.codeword.range(30, 0)) +
                   ", major_label: " + to_string(event.major_label) +
                   ", minor_label: " + to_string(event.minor_label) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const MW_DIO_event& event) {
    return os << to_string(event);
}

void sc_trace(sc_core::sc_trace_file* tf, const MW_DIO_event& event, const std::string& name) {
    // sc_trace(tf, event.codeword[31], name + ".trigger");
    // sc_trace(tf, event.codeword.range(30, 0), name + ".codeword");
    sc_trace(tf, event.codeword, name + ".codeword");
    sc_trace(tf, event.major_label, name + ".major_label");
    sc_trace(tf, event.minor_label, name + ".minor_label");
}

// --------------------------------------------------------------------------------------------
// VSM event
// --------------------------------------------------------------------------------------------
VSM_event::VSM_event()
    : Device_event_base() {
    mask     = 0;
    condbits = 0;
}

bool operator==(const VSM_event& a, const VSM_event& b) {
    return ((a.mask == b.mask) && (a.condbits == b.condbits) && (a.major_label == b.major_label) &&
            (a.minor_label == b.minor_label));
}

string to_string(const VSM_event& event) {
    string _str_ = "(mask:" + to_string(event.mask) + ", condbits: " + to_string(event.condbits) +
                   ", major_label: " + to_string(event.major_label) +
                   ", minor_label: " + to_string(event.minor_label) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const VSM_event& event) {
    return os << to_string(event);
}

void sc_trace(sc_core::sc_trace_file* tf, const VSM_event& event, const std::string& name) {
    sc_trace(tf, event.mask, name + ".mask");
    sc_trace(tf, event.condbits, name + ".condbits");
    sc_trace(tf, event.major_label, name + ".major_label");
    sc_trace(tf, event.minor_label, name + ".minor_label");
}

// --------------------------------------------------------------------------------------------
// Measurement DIO event
// --------------------------------------------------------------------------------------------
MSMT_DIO_event::MSMT_DIO_event()
    : Device_event_base() {
    condbits = 0;
    valid    = 0;
}

bool operator==(const MSMT_DIO_event& a, const MSMT_DIO_event& b) {
    return ((a.condbits == b.condbits) && (a.valid == b.valid) &&
            (a.major_label == b.major_label) && (a.minor_label == b.minor_label));
}

string to_string(const MSMT_DIO_event& event) {
    string _str_ = "(Condbits:" + to_string(event.condbits) + ", valid: " + to_string(event.valid) +
                   ", major_label: " + to_string(event.major_label) +
                   ", minor_label: " + to_string(event.minor_label) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const MSMT_DIO_event& event) {
    return os << to_string(event);
}

void sc_trace(sc_core::sc_trace_file* tf, const MSMT_DIO_event& event, const std::string& name) {
    sc_trace(tf, event.condbits, name + ".cond_bits");
    sc_trace(tf, event.valid, name + ".valid");
    sc_trace(tf, event.major_label, name + ".major_label");
    sc_trace(tf, event.minor_label, name + ".minor_label");
}

// --------------------------------------------------------------------------------------------
// Flux DIO event
// --------------------------------------------------------------------------------------------

Flux_DIO_event::Flux_DIO_event()
    : Device_event_base() {
    condbits = 0;
    codeword = 0;
}

bool operator==(const Flux_DIO_event& a, const Flux_DIO_event& b) {
    return ((a.condbits == b.condbits) && (a.codeword == b.codeword) &&
            (a.major_label == b.major_label) && (a.minor_label == b.minor_label));
}

string to_string(const Flux_DIO_event& event) {
    string _str_ = "(Condbits:" + to_string(event.condbits) +
                   ", flux codeword for corresponding qubits: " + to_string(event.codeword) +
                   ", major_label: " + to_string(event.major_label) +
                   ", minor_label: " + to_string(event.minor_label) + ")";
    return _str_;
}

std::ostream& operator<<(std::ostream& os, const Flux_DIO_event& event) {
    return os << to_string(event);
}

void sc_trace(sc_core::sc_trace_file* tf, const Flux_DIO_event& event, const std::string& name) {
    sc_trace(tf, event.condbits, name + ".cond_bits");
    sc_trace(tf, event.codeword, name + ".codeword");
    sc_trace(tf, event.major_label, name + ".major_label");
    sc_trace(tf, event.minor_label, name + ".minor_label");
}

}  // end of namespace cactus
