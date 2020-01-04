
//#define NUM_QUBITS                        Configuration_driver::num_qubits

#define INSN_WIDTH 32
#define MEMORY_ADDRESS_WIDTH 21
#define OPCODE_WIDTH 6
#define REG_FILE_WIDTH 32
#define REG_FILE_NUM 32

// configurable instruction cache
//#define G_PC_REGISTER                   1
//#define G_MEM_OUT_REG                   1

// configurable ic slice
#define G_CMD_SLICE 0
#define G_READY_SLICE 1
#define G_DATA_SLICE 1

// configurable pc register
#define G_PC_REGISTER 1

// configurable memory register
#define G_MEM_OUT_REG 1

// size of instruction cache
#define CACHE_SIZE 1000000

// size of data memory is set 256*1024*4 byte
#define DATA_MEMORY_SIZE (256 * 1024)

// configurable measurement register file
#define G_INTERLOCK_COUNTER_BITS 6

// length of a QISA instruction
#define QISA_WIDTH 32
// single/multi-instruction indicator width
#define INDICATOR_WIDTH 1

// entire length of a single instruction, excluding the indication bit
#define S_INSN_WIDTH 31
// single-instruction opecode width
#define S_OPCODE_WIDTH 6
// width of remaining bits
#define REMAINING_WIDTH (S_INSN_WIDTH - S_OPCODE_WIDTH)

// multi-instruction opecode width
#define M_OPCODE_WIDTH 8
// mask register address width
#define S_REG_ADDR_WIDTH 5
#define T_REG_ADDR_WIDTH 6
#define REG_ADDR_WIDTH 6
#define S_REG_NUM 32
#define T_REG_NUM 64

// number of multi-instructions per QISA instruction
#define VLIW_WIDTH 2
// end of bundle width
#define EOB_WIDTH 3

// number of qubits
//#define NUM_QUBITS                    Configuration_driver::num_qubits

// number of bits to store measurement result
#define G_NUM_QUBITS_LOG2 3

// number of possible nearest-neighbor interactions routines
#define NUM_NN_EDGE 24

// number of directed NN interaction
#define NUM_DIR_NN_EDGE (NUM_NN_EDGE * 2)

// number of msk bits
#define NUM_MASK_BITS 16

// number of bits of the address mask
#define ADDR_MASK_WIDTH NUM_DIR_NN_EDGE

// number of bits of mask pos
#define MASK_POS_WIDTH 3

// The bitwidth of waiting time accepted by the label manager
#define WAIT_TIME_WIDTH 20

/************** Microinstruction and micro-operation  **************/
// Microinstruction structure: (op_left, op_right, cycles, EoI)

// The width of the codeword in operations in microinstructions.
#define MAX_CODEWORD_WIDTH 8

// The width of the operations in microinstructions.
// The extra two bits are used to indicate operation type:
//      0: no-operation
//      1: micro-wave pulse
//      2: flux pulse
//      3: measurement
#define OPERATION_TYPE_WIDTH 2
#define U_INSN_OP_WIDTH (MAX_CODEWORD_WIDTH + OPERATION_TYPE_WIDTH)

// the bit width of eob used in the microcode
#define NUM_BIT_U_INSN_EOB 3

// label width
#define MAJOR_LABEL_WIDTH 6
#define MINOR_LABEL_WIDTH 4

// The bitwidth of the operation select signal in the address resolver
#define ADDR_RSV_OP_SEL_WIDTH 2

#define NUM_QISA_INSNS 256
#define NUM_U_INSN_PER_QISA 8
#define CONTROL_STORE_SIZE (256 * 8)

// specific hardware configuration
#define DIO_WIDTH 32
//#define VSM_MASK_WIDTH                  NUM_QUBITS

#define COND_WIDTH 2
#define NUM_QUBITS_FEEDLINE 10
#define MSMT_COND_WIDTH NUM_QUBITS_FEEDLINE* COND_WIDTH
#define MSMT_COND_VALID_WIDTH NUM_QUBITS_FEEDLINE + MSMT_COND_WIDTH

#define FLUX_SINGLE_CW_WIDTH 3
#define FLUX_CW_WIDTH FLUX_SINGLE_CW_WIDTH * 8
#define FLUX_COND_WIDTH COND_WIDTH * 8
#define NUM_QUBITS_SINGLE_FLUX 8

//#define FLUX_QUOTIENT                 NUM_QUBITS/8
//#define NUM_FLUX_EVENTS                   FLUX_QUOTIENT + 1
//#define FLUX_REMAINDER                    NUM_QUBITS%8

#define INSN_DEPTH 100000

#define MASK_INSN_DEPTH 100
#define VLIW_INSN_DEPTH 6

#define EXE_CYCLES_INTERNAL 400
#define EXE_CYCLES_50M EXE_CYCLES_INTERNAL / 4

#define OPCODE_QNOPR 0b110000
#define OPCODE_QNOP 0b111000
#define OPCODE_SMIS 0b100000
#define OPCODE_SMIT 0b101000
