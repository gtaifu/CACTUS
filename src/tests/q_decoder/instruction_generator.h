#include <map>
#include <systemc>

#include "generic_if.h"
#include "logger_wrapper.h"
#include "num_util.h"
#include "qasm_instruction.h"
#include "telf_module.h"

namespace cactus {

using sc_core::sc_in;
using sc_core::sc_out;
using sc_dt::sc_uint;

typedef struct quantum_instruction {
    unsigned int insn_code;
    unsigned int wait_time;
    bool         valid;
} Insn;

// stimulate
class Instruction_generator : public Telf_module {
  public:
    sc_in<bool> in_clock;

    sc_out<Qasm_instruction> out_bundle;
    sc_out<bool>             out_valid;
    sc_out<sc_uint<32>>      out_rs_wait_time;

  public:
    Insn output_insn[10] = {
        { 0x40800028, 0, true },   // SMIS s8, {3, 5}
        { 0x00000000, 0, false },  // NOP
        { 0x50208100, 0, true },   // SMIT t2, {(0, 2), (4, 6)}
        { 0x80000241, 0, true },   // bs 1 H s8
        { 0x80010010, 0, true },   // bs 0 CZ_1 t2
        { 0x80000242, 0, true },   // bs 2 H s8
        { 0x80000c41, 0, true },   // bs 1 MeasZ s8
        { 0x6000000a, 0, true },   // QWAIT 10
        { 0x70000000, 10, true },  // QWAITR 10
        { 0x00000000, 0, false }   // NOP
    };

  public:
    std::string                         file_in_asm;
    std::vector<std::string>            asm_instruction;
    std::map<std::string, unsigned int> map_label;
    Instruction_type                    m_instruction_type;

  public:
    void config();

    void read_asm_file(const std::string& qisa_asm_fn);

    void output();

    Instruction_generator(const sc_core::sc_module_name& n);

    SC_HAS_PROCESS(Instruction_generator);
};

}  // namespace cactus