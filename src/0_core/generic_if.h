/** generic_if.h
 *
 * This file defines the generic interface which can be used to connect different layters through
 * the entire quantum pipeline.
 *
 * Currently, the interface supports the description of operation & address.
 *
 * Should timing specification be added into this interface?
 *
 */

#ifndef _GENERIC_IF_H_
#define _GENERIC_IF_H_

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <systemc>
#include <vector>

namespace cactus {

/** todo: all data type in the simulator should contain two kinds of representation:
 *   - One flexible format, which is suitable for easy programming;
 *   - One binary format, which can be used to estimate the hardware resource.
 *
 *   Maybe one class/method should be provided to convert flexible format into binary format and to
 *   perform hardware resource estimation.
 */

// An unsigned integer type with a configurable width in binary representation.
// When it is of interest, the width can be used to extract statistical information.
// Currently, the maximum allowed width is 64
// TODO:  1. to check if unsigned int is sufficient or not;
//            if not, how to support signed int?
//        2. add other overloading functions to enable easy use, such as []
class Sim_uint {
  public:
    uint64_t value;
    size_t   width;

  public:
    uint64_t get_value() { return value; }
    size_t   get_width() { return width; }

    // overload of operator =
    Sim_uint& operator=(const Sim_uint& other) {
        if (this != &other) {
            value = other.value;
            width = other.width;
        }
        return *this;
    }

    // overload of operator =
    Sim_uint& operator=(unsigned int _value) {
        // TODO: check if the width of _value is larger than width or not
        value = _value;
        return *this;
    }

    // overload of operator ==
    bool operator==(const Sim_uint& other) const {
        return (value == other.value) && (width == other.width);
    }

  public:
    // default constructor
    Sim_uint() {
        value = 0;
        width = 64;
    }

    Sim_uint(size_t _width) {
        width = _width;
        value = 0;
    }
};

// TODO: add support for pulse representation
class pulse_t {
  public:
    std::vector<double> samples;

  public:
    pulse_t() { samples.clear(); }

    void init() { samples.clear(); }

    // overload of operator ==
    bool operator==(const pulse_t& pulse) const { return (samples == pulse.samples); }

    // overload of operator =
    pulse_t& operator=(const pulse_t& pulse) {
        if (this != &pulse) {
            samples.assign(pulse.samples.begin(), pulse.samples.end());
        }
        return *this;
    }
};

// TODO: add support for the matrix type
class row_t {
  public:
    std::vector<double> r;

  public:
    row_t() { r.clear(); }

    void init() { r.clear(); }

    // overload of operator ==
    bool operator==(const row_t& row) const { return (r == row.r); }

    // overload of operator =
    row_t& operator=(const row_t& row) {
        if (this != &row) {
            r.assign(row.r.begin(), row.r.end());
        }
        return *this;
    }
};

class cmatrix_t {
  public:
    std::vector<row_t> m;

  public:
    cmatrix_t() { m.clear(); }

    void init() { m.clear(); }

    // overload of operator ==
    bool operator==(const cmatrix_t& matrix) const { return (m == matrix.m); }

    // overload of operator =
    cmatrix_t& operator=(const cmatrix_t& matrix) {
        if (this != &matrix) {
            m.assign(matrix.m.begin(), matrix.m.end());
        }
        return *this;
    }
};

// --------------------------------------------------------------------------------------------
// Operation Information
// --------------------------------------------------------------------------------------------

#define QOP_HANDLER_OFFSET 0
#define QOP_CONTENT_OFFSET 16

// currently, the operation can be only represneted using a single information,
// which might be required to support more than one representation.
enum bare_qop_type {
    REPR_UNDEF    = 0,
    REPR_NAME     = (0x1 << QOP_HANDLER_OFFSET),
    REPR_OPCODE   = (0x2 << QOP_HANDLER_OFFSET),
    REPR_CODEWORD = (0x3 << QOP_HANDLER_OFFSET),
    REPR_MATRIX   = (0x1 << QOP_CONTENT_OFFSET),
    REPR_PULSE    = (0x2 << QOP_CONTENT_OFFSET)
};

// Qauntum operation without address or timing information. This is why it is called bare.
class Bare_qop {
  public:
    bare_qop_type type;

    // handlers
    std::string name;
    Sim_uint    opcode;
    Sim_uint    codeword;

    // content
    pulse_t   pulse;
    cmatrix_t matrix;

  public:
    // default constructor
    Bare_qop() {
        type     = REPR_UNDEF;
        name     = "";
        opcode   = 0;
        codeword = 0;
        pulse.init();
        matrix.init();
    }

    // reset member variables
    void reset() {
        type     = REPR_UNDEF;
        name     = "";
        opcode   = 0;
        codeword = 0;
        pulse.init();
        matrix.init();
    }

    // overload of operator =
    Bare_qop& operator=(const Bare_qop& qop) {
        if (this != &qop) {
            type = qop.type;
            switch (type) {
                case REPR_NAME:
                    name = qop.name;
                    break;

                case REPR_OPCODE:
                    opcode = qop.opcode;
                    break;

                case REPR_CODEWORD:
                    codeword = qop.codeword;
                    break;
                case REPR_PULSE:
                    pulse = qop.pulse;
                    break;
                case REPR_MATRIX:
                    matrix = qop.matrix;
                    break;

                default:
                    break;
            }
        }
        return *this;
    };
    // Bare_qop(const Bare_qop& qop)    // copy constructor

    // overload of operator ==
    bool operator==(const Bare_qop& qop) const {

        if (type != qop.type) return false;

        switch (type) {
            case REPR_NAME:
                return (name == qop.name);

            case REPR_OPCODE:
                return (opcode == qop.opcode);

            case REPR_CODEWORD:
                return (codeword == qop.codeword);

            case REPR_MATRIX:
                return (matrix == qop.matrix);

            case REPR_PULSE:
                return (pulse == qop.pulse);

            default:
                return true;
        }
    }
};

// --------------------------------------------------------------------------------------------
// Address Information
// --------------------------------------------------------------------------------------------

// DIRECT: the direct qubit or qubit tuple list
// INDIRECT_REG_NUM: the number of the target indirect address register
// INDIRECT_REG_CONTENT: register addressing, registers store qubits list, not address mask
// INDIRECT_CONTENT: the content of the indirect address HARDWIRE: the
// information in the single/multi_qubit_addr_info is not used.
enum content_type_t {
    UNDEF,
    DIRECT,
    INDIRECT_REG_NUM,
    INDIRECT_REG_CONTENT,
    INDIRECT_CONTENT,
    HARDWIRE
};

// SINGLE: the address used by single-qubit operations
// MULTIPLE: the address used by n-qubit operations, where n >= 2.
enum num_tgt_qubits_type_t { SINGLE, MULTIPLE };

class Q_addr_type {
  public:
    content_type_t        c_type;
    num_tgt_qubits_type_t q_num_type;

  public:
    // default constructor
    Q_addr_type() {
        c_type     = UNDEF;
        q_num_type = SINGLE;
    }

    void reset() {
        c_type     = UNDEF;
        q_num_type = SINGLE;
    }

    // overload of operator =
    Q_addr_type& operator=(const Q_addr_type& type) {
        if (this != &type) {
            c_type     = type.c_type;
            q_num_type = type.q_num_type;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Q_addr_type& addr_type) const {
        return (c_type == addr_type.c_type) && (q_num_type == addr_type.q_num_type);
    }
};

class Single_qubit_addr_info {
  public:
    Q_addr_type* const type;

    // max number of qubits that can be operated by one operation in SOMQ
    size_t somq_width;

    // directly stores the indices of qubits
    std::vector<size_t> qubit_indices;

    // use each bit in the mask to indicate if the qubit is selected or not
    Sim_uint mask;

  public:
    // default constructor
    Single_qubit_addr_info(Q_addr_type* _type)
        : type(_type) {

        somq_width = 1;
        qubit_indices.clear();
    }

    // reset member variables
    void reset() {
        mask = 0;
        qubit_indices.clear();
    }

    // overload of operator =
    Single_qubit_addr_info& operator=(const Single_qubit_addr_info& single_addr) {
        if (this != &single_addr) {
            somq_width = single_addr.somq_width;
            qubit_indices.assign(single_addr.qubit_indices.begin(),
                                 single_addr.qubit_indices.end());
            mask = single_addr.mask;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Single_qubit_addr_info& single_addr) const {
        return (somq_width == single_addr.somq_width) &&
               (qubit_indices == single_addr.qubit_indices) && (mask == single_addr.mask);
    }
};

class Multi_qubit_addr_info {
  public:
    // the type is constantly pointed to the type variable in Q_tgt_addr
    // the pointer itself cannot be modified
    // however, the data pointed by the pointer can be modified.
    Q_addr_type* const type;

    // max number of qubit tuples that can be operated by one operation in SOMQ
    size_t somq_width;

    // a list of qubit tuples, used by n-qubit gates, n >= 2
    std::vector<std::vector<size_t>> qubit_tuples;

    // use each bit in the mask to indicate if a qubit tuple is selected or not
    Sim_uint mask;

  public:
    // default constructor
    Multi_qubit_addr_info(Q_addr_type* _type)
        : type(_type) {

        somq_width = 1;
        qubit_tuples.clear();
    }

    void reset() {
        mask = 0;
        qubit_tuples.clear();
    }

    // overload of operator =
    Multi_qubit_addr_info& operator=(const Multi_qubit_addr_info& multi_addr) {
        if (this != &multi_addr) {
            somq_width = multi_addr.somq_width;
            qubit_tuples.assign(multi_addr.qubit_tuples.begin(), multi_addr.qubit_tuples.end());
            mask = multi_addr.mask;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Multi_qubit_addr_info& multi_addr) const {
        return (somq_width == multi_addr.somq_width) && (qubit_tuples == multi_addr.qubit_tuples) &&
               (mask == multi_addr.mask);
    }
};

// a universal data structure which stores the address of quantum operations
class Q_tgt_addr {
  public:
    // the type of address
    // when it is HARDWIRE, the content inside this data structure is not used.
    Q_addr_type type;

    // the number of register which stores the indrect address
    // For single-qubit operations, each indrect address can specify a list of qubits
    // For n-quibit gates, each indrect address can specify a list of n-qubit tuples
    // Currently, the indirect address is the mask.
    Sim_uint indirect_addr_reg_num;

    // the address information for single-qubit operations
    Single_qubit_addr_info sq_op_addr;

    // the address information for multiple-qubit operations
    Multi_qubit_addr_info mq_op_addr;

  public:
    // default constructor
    Q_tgt_addr()
        : type()
        , sq_op_addr(&type)
        , mq_op_addr(&type) {}

    // reset member variables
    void reset() {
        indirect_addr_reg_num = 0;
        type.reset();
        sq_op_addr.reset();
        mq_op_addr.reset();
    }

    // overload of operator =
    Q_tgt_addr& operator=(const Q_tgt_addr& addr) {
        if (this != &addr) {
            type                  = addr.type;
            indirect_addr_reg_num = addr.indirect_addr_reg_num;
            sq_op_addr            = addr.sq_op_addr;
            mq_op_addr            = addr.mq_op_addr;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Q_tgt_addr& addr) const {
        return (type == addr.type) && (indirect_addr_reg_num == addr.indirect_addr_reg_num) &&
               (sq_op_addr == addr.sq_op_addr) && (mq_op_addr == addr.mq_op_addr);
    }
};

// quantum register
class Q_mask_reg {
  public:
    // default 32 single-qubit operation registers
    Sim_uint                         s_reg_mask[32] = { 0 };
    std::vector<std::vector<size_t>> s_reg_content;  // used for asm instruction

    // default 32 multiple-qubit operation registers
    Sim_uint                                      m_reg_mask[32] = { 0 };
    std::vector<std::vector<std::vector<size_t>>> m_reg_content;  // used for asm instruction

  public:
    // default constructor
    Q_mask_reg() {
        s_reg_content.resize(32);
        m_reg_content.resize(32);
    }

    // reset member variables
    void reset() {
        for (size_t i = 0; i < 32; ++i) {
            s_reg_mask[i] = 0;
            m_reg_mask[i] = 0;
            s_reg_content[i].clear();
            s_reg_content[i].clear();
        }
    }

    // write register
    void set_reg_mask(const Sim_uint& mask, const Sim_uint& reg_num,
                      enum num_tgt_qubits_type_t type) {
        if (type == SINGLE) {
            s_reg_mask[reg_num.value] = mask;
        } else {
            m_reg_mask[reg_num.value] = mask;
        }
    }

    // read register
    Sim_uint get_reg_mask(const Sim_uint& reg_num, enum num_tgt_qubits_type_t type) {
        if (type == SINGLE) {
            return s_reg_mask[reg_num.value];
        } else {
            return m_reg_mask[reg_num.value];
        }
    }

    // write single-qubit register
    void set_s_reg_content(const std::vector<size_t>& content, const Sim_uint& reg_num) {
        s_reg_content[reg_num.value].assign(content.begin(), content.end());
    }

    // write multiple-qubit register
    void set_m_reg_content(const std::vector<std::vector<size_t>>& content,
                           const Sim_uint&                         reg_num) {
        m_reg_content[reg_num.value].assign(content.begin(), content.end());
    }

    // read single-qubit register
    std::vector<size_t> get_s_reg_content(const Sim_uint& reg_num) {
        return s_reg_content[reg_num.value];
    }

    // read multiple-qubit register
    std::vector<std::vector<size_t>> get_m_reg_content(const Sim_uint& reg_num) {
        return m_reg_content[reg_num.value];
    }
};

// --------------------------------------------------------------------------------------------
// Timing Information
// --------------------------------------------------------------------------------------------

// NONE:         the timing information is not specified by this Timing_info field.
// WAIT_TIME:    the timing is directly specified by the waiting time,
//                  usually used by a quantum bundle.
// TIMING_LABEL: the timing is indirectly specified by the timing label,
//                  usually used by individual quantum operations
// TIMING_POINT: this is a timing point, which contains a label and a waiting time.

enum timing_type_t { NONE, WAIT_TIME, TIMING_LABEL, TIMING_POINT };

// The timing information corresponding to a quantum bundle, or a quantum operation
class Timing_info {
  public:
    timing_type_t type;
    unsigned int  wait_time;
    unsigned int  label;

  public:
    // default constructor
    Timing_info() {
        type      = NONE;
        wait_time = 0;
        label     = 0;
    }

    // reset member variables
    void reset() {
        type      = NONE;
        wait_time = 0;
        label     = 0;
    }

    // overload of operator =
    Timing_info& operator=(const Timing_info& timing) {
        if (this != &timing) {
            type      = timing.type;
            wait_time = timing.wait_time;
            label     = timing.label;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Timing_info& timing) const {
        if (type != timing.type) return false;

        switch (type) {
            case WAIT_TIME:
                return (wait_time == timing.wait_time);

            case TIMING_LABEL:
                return (label == timing.label);

            case TIMING_POINT:
                return (label == timing.label) && (wait_time == timing.wait_time);

            default:
                return true;
        }
    }
};

// --------------------------------------------------------------------------------------------
// General Quantum Operation
// --------------------------------------------------------------------------------------------
// a quantum operation which can contain all information: operation, target address, timing.
class Fledged_qop {
  public:
    Timing_info timing;
    Bare_qop    op;
    Q_tgt_addr  addr;

    // reset member variables
    void reset() {
        addr.reset();
        op.reset();
        timing.reset();
    }

    // operation valid or not
    bool is_valid() const { return op.type != NONE; }

    // overload of operator =
    Fledged_qop& operator=(const Fledged_qop& qop) {
        if (this != &qop) {
            timing = qop.timing;
            op     = qop.op;
            addr   = qop.addr;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Fledged_qop& qop) const {
        return (timing == qop.timing) && (op == qop.op) && (addr == qop.addr);
    }
};

// --------------------------------------------------------------------------------------------
// The General Pipeline Interface
// --------------------------------------------------------------------------------------------

// The inerface content-valid bit
// An interface can contain up to the three kinds of information:
//   1. waiting time
//   2. quantum operation + target address
//   3. Content to update the indirect address register when indirect is used.
class If_content_type {
  public:
    bool valid_wait;
    bool valid_qop;
    bool valid_set_addr;

  public:
    // default constructor
    If_content_type() {
        valid_wait     = false;
        valid_qop      = false;
        valid_set_addr = false;
    }

    // reset member variables
    void reset() {
        valid_wait     = false;
        valid_qop      = false;
        valid_set_addr = false;
    }

    // overload of operator =
    If_content_type& operator=(const If_content_type& content_type) {
        if (this != &content_type) {
            valid_wait     = content_type.valid_wait;
            valid_qop      = content_type.valid_qop;
            valid_set_addr = content_type.valid_set_addr;
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const If_content_type& content_type) const {
        return (valid_wait == content_type.valid_wait) && (valid_qop == content_type.valid_qop) &&
               (valid_set_addr == content_type.valid_set_addr);
    }
};

class Q_pipe_interface {
  public:
    // indication of the interface content
    If_content_type if_content;

    // information used to update the indirect address register
    std::vector<Q_tgt_addr> addrs_to_set;

    // timing information
    Timing_info timing;

    // If address type is HARDWIRE, the address of each operation
    //    is the index of the operation in the ops vecotr.
    Q_addr_type              type;
    std::vector<Fledged_qop> ops;

    // Xiang is not sure about whether this configuration should be here or not.
    unsigned int vliw_width;

  public:
    Q_pipe_interface() { if_content = If_content_type(); }

    // reset member variables
    void reset() {
        if_content.reset();
        timing.reset();
        addrs_to_set.clear();
        type.reset();
        ops.clear();
    }

    // overload of operator =
    Q_pipe_interface& operator=(const Q_pipe_interface& q_pipe_interface) {
        if (this != &q_pipe_interface) {
            if_content = q_pipe_interface.if_content;
            timing     = q_pipe_interface.timing;
            type       = q_pipe_interface.type;
            vliw_width = q_pipe_interface.vliw_width;
            addrs_to_set.assign(q_pipe_interface.addrs_to_set.begin(),
                                q_pipe_interface.addrs_to_set.end());
            ops.assign(q_pipe_interface.ops.begin(), q_pipe_interface.ops.end());
        }
        return *this;
    }

    // overload of operator ==
    bool operator==(const Q_pipe_interface& q_pipe_interface) const {
        return (if_content == q_pipe_interface.if_content) && (timing == q_pipe_interface.timing) &&
               (type == q_pipe_interface.type) && (addrs_to_set == q_pipe_interface.addrs_to_set) &&
               (ops == q_pipe_interface.ops) && (vliw_width == q_pipe_interface.vliw_width);
    }
};

// output stream If_content_type
inline std::ostream& operator<<(std::ostream& os, const If_content_type& type) {
    if (type.valid_qop) {
        os << std::setfill(' ') << std::setw(7) << "qop"
           << " ";
        return os;
    }
    if (type.valid_wait) {
        os << std::setfill(' ') << std::setw(7) << "wait"
           << " ";
        return os;
    }
    if (type.valid_set_addr) {
        os << std::setfill(' ') << std::setw(7) << "addr"
           << " ";
        return os;
    }

    os << std::setfill(' ') << std::setw(7) << "error"
       << " ";
    return os;
}

// output stream Bare_qop
inline std::ostream& operator<<(std::ostream& os, const Bare_qop& qop) {
    std::stringstream op_code;
    op_code << "0x" << std::hex << qop.opcode.value;

    std::stringstream codeword;
    codeword << "0x" << std::hex << qop.codeword.value;

    switch (qop.type) {  // TODO:
        case REPR_NAME:
            os << std::setfill(' ') << std::setw(8) << "name"
               << " " << std::setfill(' ') << std::setw(10) << qop.name << " ";
            break;
        case REPR_CODEWORD:
            os << std::setfill(' ') << std::setw(8) << "codeword"
               << " " << std::setfill(' ') << std::setw(10) << codeword.str() << " ";
            break;
        case REPR_OPCODE:
            os << std::setfill(' ') << std::setw(8) << "opcode"
               << " " << std::setfill(' ') << std::setw(10) << op_code.str() << " ";
            break;
        case REPR_MATRIX:
            os << std::setfill(' ') << std::setw(8) << "matrix"
               << " " << std::setfill(' ') << std::setw(10) << " "
               << " ";
            break;
        case REPR_PULSE:
            os << std::setfill(' ') << std::setw(8) << "pulse"
               << " " << std::setfill(' ') << std::setw(10) << " "
               << " ";
            break;

        default:
            os << std::setfill(' ') << std::setw(8) << "undef"
               << " " << std::setfill(' ') << std::setw(10) << " "
               << " ";
            break;
    }

    return os;
}

// output stream Q_addr_type
inline std::ostream& operator<<(std::ostream& os, const Q_addr_type& addr_type) {
    // addressing type
    switch (addr_type.c_type) {
        case DIRECT:
            os << std::setfill(' ') << std::setw(9) << "direct"
               << " ";
            break;

        case INDIRECT_REG_NUM:
            os << std::setfill(' ') << std::setw(9) << "reg_num"
               << " ";
            break;

        case INDIRECT_REG_CONTENT:
            os << std::setfill(' ') << std::setw(9) << "reg_cont"
               << " ";
            break;

        case INDIRECT_CONTENT:
            os << std::setfill(' ') << std::setw(9) << "content"
               << " ";
            break;

        case HARDWIRE:
            os << std::setfill(' ') << std::setw(9) << "hardwire"
               << " ";
            break;

        default:
            os << std::setfill(' ') << std::setw(9) << "undef"
               << " ";
            break;
    }

    // single or multi qubit operation
    switch (addr_type.q_num_type) {
        case SINGLE:
            os << std::setfill(' ') << std::setw(6) << "single"
               << " ";
            break;
        case MULTIPLE:
            os << std::setfill(' ') << std::setw(6) << "multi"
               << " ";
            break;

        default:
            os << std::setfill(' ') << std::setw(6) << "error"
               << " ";
            break;
    }
    return os;
}

// output stream Q_tgt_addr
inline std::ostream& operator<<(std::ostream& os, const Q_tgt_addr& addr) {

    std::stringstream reg_num;
    std::stringstream mask;
    std::stringstream qubit_indice;
    std::stringstream qubit_tuple;

    // addressing type
    os << addr.type;

    if (addr.type.c_type == INDIRECT_REG_NUM) {
        // register number
        reg_num << "0x" << std::hex << addr.indirect_addr_reg_num.value;
        os << std::setfill(' ') << std::setw(10) << reg_num.str() << " ";

        // single operation mask
        if (addr.type.q_num_type == SINGLE) {
            mask << "0x" << std::hex << addr.sq_op_addr.mask.value;
            os << std::setfill(' ') << std::setw(10) << mask.str() << " ";
        } else {
            // multi qubits operation mask
            mask << "0x" << std::hex << addr.mq_op_addr.mask.value;
            os << std::setfill(' ') << std::setw(10) << mask.str() << " ";
        }

        // qubit indice or qubit tuple
        os << std::setfill(' ') << std::setw(24) << " "
           << " ";
    } else {
        if (addr.type.c_type == INDIRECT_REG_CONTENT) {  // reg_num
            reg_num << "0x" << std::hex << addr.indirect_addr_reg_num.value;
            os << std::setfill(' ') << std::setw(10) << reg_num.str() << " ";
        } else {
            os << std::setfill(' ') << std::setw(10) << " "
               << " ";
        }

        os << std::setfill(' ') << std::setw(10) << " "
           << " ";  // mask

        // single operation qubit indice
        if (addr.type.q_num_type == SINGLE) {
            for (unsigned int j = 0; j < addr.sq_op_addr.qubit_indices.size(); ++j) {
                qubit_indice << addr.sq_op_addr.qubit_indices[j] << " ";
            }
            os << std::setfill(' ') << std::setw(24) << qubit_indice.str() << " ";
        } else {
            // multi qubits operation qubit tuple
            for (unsigned int j = 0; j < addr.mq_op_addr.qubit_tuples.size(); ++j) {
                qubit_tuple << "{";
                for (unsigned int k = 0; k < addr.mq_op_addr.qubit_tuples[j].size(); ++k) {
                    qubit_tuple << addr.mq_op_addr.qubit_tuples[j][k];
                    if (k != addr.mq_op_addr.qubit_tuples[j].size() - 1) {
                        qubit_tuple << " ";
                    }
                }
                qubit_tuple << "} ";
            }
            os << std::setfill(' ') << std::setw(24) << qubit_tuple.str() << " ";
        }
    }

    return os;
}

// output stream Timing_info
inline std::ostream& operator<<(std::ostream& os, const Timing_info& timing) {
    // timing info
    std::stringstream wait_time;

    std::stringstream label;

    switch (timing.type) {
        case WAIT_TIME:
            wait_time << "0x" << std::hex << timing.wait_time;
            os << std::setfill(' ') << std::setw(9) << "time"
               << " " << std::setfill(' ') << std::setw(9) << wait_time.str() << " "
               << std::setfill(' ') << std::setw(5) << ""
               << " ";
            break;
        case TIMING_LABEL:
            label << "0x" << std::hex << timing.label;
            os << std::setfill(' ') << std::setw(9) << "label"
               << " " << std::setfill(' ') << std::setw(9) << ""
               << " "
               << " " << std::setfill(' ') << std::setw(5) << label.str() << " ";
            break;
        case TIMING_POINT:
            wait_time << "0x" << std::hex << timing.wait_time;
            label << "0x" << std::hex << timing.label;
            os << std::setfill(' ') << std::setw(9) << "point"
               << " " << std::setfill(' ') << std::setw(9) << wait_time.str() << " "
               << std::setfill(' ') << std::setw(5) << label.str() << " ";
            break;
        default:
            os << std::setfill(' ') << std::setw(9) << "NONE"
               << " " << std::setfill(' ') << std::setw(9) << ""
               << " " << std::setfill(' ') << std::setw(5) << ""
               << " ";
            break;
    }

    return os;
}

// output stream Fledged_qop
inline std::ostream& operator<<(std::ostream& os, const Fledged_qop& fledged_qop) {

    if (fledged_qop.is_valid()) {
        os << fledged_qop.addr;
        os << fledged_qop.op;
    }

    return os;
}

// output stream Q_pipe_interface
inline std::ostream& operator<<(std::ostream& os, const Q_pipe_interface& q_pipe_interface) {

    if (q_pipe_interface.if_content.valid_qop) {
        for (size_t i = 0; i < q_pipe_interface.ops.size(); ++i) {
            if (q_pipe_interface.ops[i].is_valid()) {
                // instruction type
                os << q_pipe_interface.if_content;

                // timing info
                os << q_pipe_interface.ops[i].timing;

                // operation info
                os << q_pipe_interface.ops[i];
                os << std::endl;
            }
        }
        return os;
    }

    if (q_pipe_interface.if_content.valid_set_addr) {
        for (size_t i = 0; i < q_pipe_interface.addrs_to_set.size(); ++i) {
            // instruction type
            os << q_pipe_interface.if_content;

            // timing info
            os << q_pipe_interface.timing;

            // addr info
            os << q_pipe_interface.addrs_to_set[i];
            os << std::endl;
        }
        return os;
    }
    if (q_pipe_interface.if_content.valid_wait) {
        // instruction type
        os << q_pipe_interface.if_content;

        // timing info
        os << q_pipe_interface.timing;
        os << std::endl;
    }
    return os;
}

inline void sc_trace(sc_core::sc_trace_file* tf, const Q_pipe_interface& q_pipe_interface,
                     const std::string& name) {}

inline void sc_trace(sc_core::sc_trace_file* tf, const Timing_info& timing,
                     const std::string& name) {}

inline void sc_trace(sc_core::sc_trace_file* tf, const Fledged_qop& op, const std::string& name) {}

inline void sc_trace(sc_core::sc_trace_file* tf, const If_content_type& type,
                     const std::string& name) {}

inline void sc_trace(sc_core::sc_trace_file* tf, const Q_tgt_addr& addr, const std::string& name) {}

// --------------------------------------------------------------------------------------------
// General measurement Information
// --------------------------------------------------------------------------------------------
// Pure virtual class for measurement interface
class Measurement_if {
  public:  // interface
    virtual void reset() = 0;

    virtual void set_meas_ena(const std::vector<bool>& vec)        = 0;
    virtual void set_meas_ena_cancel(const std::vector<bool>& vec) = 0;
    virtual void set_meas_data(const std::vector<bool>& vec)       = 0;
    virtual void set_meas_data_valid(const std::vector<bool>& vec) = 0;

    virtual std::vector<bool> get_meas_ena()        = 0;
    virtual std::vector<bool> get_meas_ena_cancel() = 0;
    virtual std::vector<bool> get_meas_data()       = 0;
    virtual std::vector<bool> get_meas_data_valid() = 0;

    virtual ~Measurement_if() = default;
};

// General measurement interface derived of Measurement_if
class Generic_meas_if : public Measurement_if {
  public:
    // timing info is used for log
    Timing_info timing;

  private:
    // private variables
    std::vector<bool> meas_ena;
    std::vector<bool> meas_ena_cancel;
    std::vector<bool> meas_data;
    std::vector<bool> meas_data_valid;

  public:  // public member function
    void reset() {
        timing.reset();
        meas_data.clear();
        meas_ena.clear();
        meas_ena_cancel.clear();
        meas_data_valid.clear();
    }

    // set meas_ena
    void set_meas_ena(const std::vector<bool>& vec) { meas_ena.assign(vec.begin(), vec.end()); }

    // set meas_ena_cancel
    void set_meas_ena_cancel(const std::vector<bool>& vec) {
        meas_ena_cancel.assign(vec.begin(), vec.end());
    }

    // set_meas_data
    void set_meas_data(const std::vector<bool>& vec) { meas_data.assign(vec.begin(), vec.end()); }

    // set_meas_data_valid
    void set_meas_data_valid(const std::vector<bool>& vec) {
        meas_data_valid.assign(vec.begin(), vec.end());
    }

    // get_meas_ena
    std::vector<bool> get_meas_ena() { return meas_ena; }

    // get_meas_ena_cancel
    std::vector<bool> get_meas_ena_cancel() { return meas_ena_cancel; }

    // get_meas_data
    std::vector<bool> get_meas_data() { return meas_data; }

    // get_meas_data_valid
    std::vector<bool> get_meas_data_valid() { return meas_data_valid; }

    ~Generic_meas_if() {}

    // overload of operator =
    Generic_meas_if& operator=(const Generic_meas_if& meas) {
        if (this != &meas) {
            timing = meas.timing;
            meas_ena.assign(meas.meas_ena.begin(), meas.meas_ena.end());
            meas_ena_cancel.assign(meas.meas_ena_cancel.begin(), meas.meas_ena_cancel.end());
            meas_data.assign(meas.meas_data.begin(), meas.meas_data.end());
            meas_data_valid.assign(meas.meas_data_valid.begin(), meas.meas_data_valid.end());
        }

        return *this;
    }

    // overload of operator ==
    bool operator==(const Generic_meas_if& meas) const {
        return (meas_ena == meas.meas_ena) && (meas_ena_cancel == meas.meas_ena_cancel) &&
               (meas_data_valid == meas.meas_data_valid) && (meas_data == meas.meas_data);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Generic_meas_if& meas) { return os; }

inline void sc_trace(sc_core::sc_trace_file* tf, const Generic_meas_if& meas,
                     const std::string& name) {}

enum Qubit_simulator_type { QUANTUMSIM = 0, QICIRCUIT };
enum Instruction_type { BIN = 0, ASM };

}  // namespace cactus

#endif  // _GENERIC_IF_H_