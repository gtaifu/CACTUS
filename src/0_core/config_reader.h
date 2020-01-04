#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "cmdparser/cmdparser.h"
#include "data_memory.h"
#include "generic_if.h"
#include "interface_lib.h"
#include "json_wrapper.h"
#include "logger_wrapper.h"

namespace cactus {

class config_reader {

  public:  // constructor
    config_reader();
    ~config_reader();

  public:  // logging level for each logger.
    void init_data_memory(const std::string& data_size);
    void read_log_levels(std::string log_level_fn);

  public:  // read various configurations.
    void read_config(std::string config_file_list_fn);
    void read_from_file(std::string config_file_name);
    void read_electronic_config(std::string config_file_name);
    void read_mock_msmt_result(std::string mock_msmt_res_fn);
    void read_qubit_gate_config(std::string config_file_fn);

  public:  // set configurations default
    void set_config_default();
    void set_log_level_default();
    void set_qubit_gate_default();
    void set_topology_default();

  public:  // parser command line
    void init_cmdparser(int argc, char* argv[]);
    void run_cmdparser();
    void configure_cmdparser();

  private:
    void find_config_file(const std::string& config_file_list_fn, std::string& config_fn);

  public:  // the names of configuration files
    void read_config_file_list(std::string config_file_list_fn);

    std::string qisa_bin_fn          = "";
    std::string qisa_asm_fn          = "";
    std::string topology_fn          = "";
    std::string output_dir           = "./sim_output/";
    std::string log_level_fn         = "";
    std::string config_file_list_fn  = "";
    std::string qubit_gate_config_fn = "";
    std::string data_mem_dump_fn     = "";
    std::string mock_msmt_res_fn     = "";
    // control store,elec config,msmt res are not used in this version
    // std::string control_store_fn;
    // std::string elec_config_fn;
    // std::string msmt_res_fn;  // file which defines the mock measurement result

  public:
    // ----------------------------------------------------------------------
    // Topology-related information
    // ----------------------------------------------------------------------
    unsigned int num_qubits         = 7;
    unsigned int num_directed_edges = 0;

    // in_edges_of_qubit[i] stores all the edges whose right qubit is i.
    std::vector<std::vector<unsigned int>> in_edges_of_qubit;

    // out_edges_of_qubit[i] stores all the edges whose left qubit is i.
    std::vector<std::vector<unsigned int>> out_edges_of_qubit;

    // nearby_qubits[i] stores the nearby qubits of qubit i
    std::vector<std::vector<unsigned int>> nearby_qubits;

    // The initial frequency of each qubit
    std::vector<double> qubit_frequency;

    // ----------------------------------------------------------------------
    // Device-related information
    // ----------------------------------------------------------------------
    unsigned int num_mw_devices = 0;  // The number of microwave devices.
    // microwave channels of each device.
    std::vector<std::vector<unsigned int>> channels_of_mw_device;
    // The qubits associated with each microwave channel
    std::vector<std::vector<unsigned int>> qubits_in_each_mw_channel;

    std::vector<Microwave_device>               mw_device_array;
    std::map<size_t, std::pair<size_t, size_t>> qubit_chnnl_pos;

    unsigned int num_flux_devices         = 0;
    unsigned int num_flux_chnl_per_device = 8;

    unsigned int num_msmt_devices = 0;
    // qubits associated with feedline
    std::vector<std::vector<unsigned int>> qubits_in_each_feedline;

    // ----------------------------------------------------------------------
    // Configuration of devices in ADI
    // ----------------------------------------------------------------------
    std::vector<std::string> mw_lut_content;    // microwave AWG LUT
    std::vector<std::string> flux_lut_content;  // flux AWG LUT

    unsigned int msmt_latency = 30;  // 30 cycles, 600 ns

    // each value corresponds to one trigger, no matter what qubits are selected.
    std::vector<unsigned int> msmt_res_array;

    // ----------------------------------------------------------------------
    // Timing configuration.
    // ----------------------------------------------------------------------
    std::map<std::string, unsigned int> single_qubit_gate_time;
    std::map<std::string, unsigned int> two_qubit_gate_time;
    unsigned int                        cycle_time = 20;  // ns
    // the maximum frequency change of this flux
    std::map<std::string, double> flux_delta_freq;

    // default instruction type is binary
    Instruction_type instruction_type = Instruction_type::ASM;

    // ----------------------------------------------------------------------
    // operation representation mapping
    // ----------------------------------------------------------------------
    unsigned int                    vliw_width = 2;
    std::map<uint64_t, std::string> opcode_to_opname_lut;

    // ----------------------------------------------------------------------
    // quantum simulator configuration
    // 0 : quantum sim
    // 1 : QIcircuit sim
    // ----------------------------------------------------------------------
    // default simulator is quantumsim
    Qubit_simulator_type qubit_simulator = Qubit_simulator_type::QUANTUMSIM;

    // ----------------------------------------------------------------------
    // data memory
    // ----------------------------------------------------------------------
    Data_memory* data_memory;
    std::string  data_mem_size_str = "1M";  // 1MB default
    unsigned int dump_start_addr;
    unsigned int dump_mem_size;

    // ----------------------------------------------------------------------
    // total number of simulation cycles
    // ----------------------------------------------------------------------
    unsigned int num_sim_cycles = 3000;  // run 3000 cycles default

    // ----------------------------------------------------------------------
    // command line parser
    // ----------------------------------------------------------------------
    cli::Parser* cmdparser = nullptr;

};  // end of class layout_reader()

#include "constants.h"

}  // end of namespace cactus

#endif  //_CONFIG_READER_H_
