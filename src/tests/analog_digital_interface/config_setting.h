#ifndef _CONFIG_SETTING_H_
#define _CONFIG_SETTING_H_

#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "generic_if.h"
#include "interface_lib.h"
#include "json_wrapper.h"
#include "logger_wrapper.h"

namespace cactus {

class config_setting {

  public:  // constructor
    config_setting();

  public:  // read various configurations.
    void read_log_levels(std::string log_level_fn);
    void read_config_file_list(std::string config_file_list_fn);
    void read_opcode_to_opname_lut(std::string config_file_fn);
    void read_global_setting(std::string config_file_fn);
    void find_config_file(const std::string& config_file_list_fn, std::string& config_fn);

  public:
    std::string config_file_list_fn;
    std::string opcode_to_opname_fn;
    std::string log_level_fn;
    std::string global_setting_fn;
    std::string qisa_asm_fn;

    unsigned int num_qubits;
    unsigned int vliw_width;
    unsigned int sim_time;  // simulation time
                            // quantum simulator
    Qubit_simulator_type qubit_simulator = Qubit_simulator_type::QUANTUMSIM;
    // instruction type : assemble or binary
    Instruction_type instruction_type = Instruction_type::BIN;

  public:
    // ----------------------------------------------------------------------
    // Configuration of devices in ADI
    // ----------------------------------------------------------------------
    std::map<uint64_t, std::string> opcode_to_opname_lut_content;  // opcode <-> opname
};

}  // end of namespace cactus

#endif  //_CONFIG_SETTING_H_
