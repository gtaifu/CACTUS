#include "config_reader.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "num_util.h"

using json = nlohmann::json;

namespace cactus {

std::map<std::string, std::vector<std::vector<std::string>>> asm_gate_time_mapping_default = {
    { "single_qubit_gate",
      { { "x", "20" },
        { "rx", "20" },
        { "xm", "20" },
        { "y", "20" },
        { "ry", "20" },
        { "ym", "20" },
        { "z", "20" },
        { "rz", "20" },
        { "zm", "20" },
        { "h", "20" },
        { "s", "20" },
        { "t", "20" },
        { "tdg", "20" },
        { "sdg", "20" },
        { "mock_meas", "20" },
        { "measure", "600" } } },
    { "two_qubit_gate", { { "cz", "40" } } }
};

std::map<std::string, std::vector<std::vector<std::string>>> opcode_gate_mapping_default = {
    { "single_qubit_gate",
      { { "0", "null", "0" },   { "1", "h", "20" },        { "2", "x45", "20" },
        { "3", "xm45", "20" },  { "6", "measure", "600" }, { "8", "x90", "20" },
        { "9", "xm90", "20" },  { "10", "x180", "20" },    { "11", "xm180", "20" },
        { "12", "y90", "20" },  { "13", "ym90", "20" },    { "14", "z45", "20" },
        { "15", "zm45", "20" }, { "16", "x15", "20" },     { "17", "zm45", "20" },
        { "18", "x30", "20" },  { "19", "xm30", "20" },    { "20", "x60", "20" },
        { "21", "xm60", "20" }, { "22", "x75", "20" },     { "23", "xm75", "20" },
        { "24", "x105", "20" }, { "25", "xm105", "20" },   { "26", "z90", "20" },
        { "27", "zm90", "20" }, { "28", "y180", "20" },    { "29", "z180", "20" } } },
    { "two_qubit_gate", { { "128", "cz", "40" } } }
};

config_reader::config_reader() { set_config_default(); }

void config_reader::set_config_default() { set_topology_default(); }

void config_reader::set_log_level_default() {

    std::vector<std::string> logger;
    logger.push_back("console");
    logger.push_back("counter_registry_logger");
    logger.push_back("config_logger");
    logger.push_back("telf_logger");
    logger.push_back("cache_logger");
    logger.push_back("MRF_logger");
    logger.push_back("qsim_logger");
    logger.push_back("json_logger");
    logger.push_back("asm_logger");

    std::shared_ptr<spdlog::logger> new_logger = nullptr;
    for (auto it = logger.begin(); it != logger.end(); ++it) {
        new_logger = safe_create_logger(*it);
        new_logger->set_level(spdlog::level::from_str("error"));
    }
}

// set edges of 7 qubit simulator
void config_reader::set_topology_default() {

    unsigned int left_qubit         = 0;
    unsigned int right_qubit        = 0;
    unsigned int num_directed_edges = 16;

    in_edges_of_qubit.clear();
    out_edges_of_qubit.clear();
    in_edges_of_qubit.resize(num_directed_edges);
    out_edges_of_qubit.resize(num_directed_edges);
    for (size_t i = 0; i < in_edges_of_qubit.size(); ++i) {
        in_edges_of_qubit[i].clear();
        out_edges_of_qubit[i].clear();
    }

    std::vector<std::pair<int, int>> directed_edges;
    directed_edges.push_back(std::make_pair(2, 0));
    directed_edges.push_back(std::make_pair(0, 3));
    directed_edges.push_back(std::make_pair(3, 1));
    directed_edges.push_back(std::make_pair(1, 4));
    directed_edges.push_back(std::make_pair(2, 5));
    directed_edges.push_back(std::make_pair(5, 3));
    directed_edges.push_back(std::make_pair(3, 6));
    directed_edges.push_back(std::make_pair(6, 4));
    directed_edges.push_back(std::make_pair(0, 2));
    directed_edges.push_back(std::make_pair(3, 0));
    directed_edges.push_back(std::make_pair(1, 3));
    directed_edges.push_back(std::make_pair(4, 1));
    directed_edges.push_back(std::make_pair(5, 2));
    directed_edges.push_back(std::make_pair(3, 5));
    directed_edges.push_back(std::make_pair(6, 3));
    directed_edges.push_back(std::make_pair(4, 6));

    for (size_t i = 0; i < num_directed_edges; i++) {

        left_qubit  = directed_edges[i].first;
        right_qubit = directed_edges[i].second;

        out_edges_of_qubit[left_qubit].push_back(static_cast<unsigned int>(i));
        in_edges_of_qubit[right_qubit].push_back(static_cast<unsigned int>(i));
    }
}

void config_reader::set_qubit_gate_default() {

    // set <opcode,opname> mapping and <opname,duration_time> mapping
    std::vector<std::vector<std::string>> single_qubit_gate_config;
    std::vector<std::vector<std::string>> two_qubit_gate_config;
    switch (instruction_type) {
        case Instruction_type::ASM:
            single_qubit_gate_config = asm_gate_time_mapping_default["single_qubit_gate"];
            two_qubit_gate_config    = asm_gate_time_mapping_default["two_qubit_gate"];
            break;

        case Instruction_type::BIN:
            single_qubit_gate_config = opcode_gate_mapping_default["single_qubit_gate"];
            two_qubit_gate_config    = opcode_gate_mapping_default["two_qubit_gate"];
            break;

        default:
            break;
    }

    if (instruction_type == Instruction_type::BIN) {
        for (size_t i = 0; i < single_qubit_gate_config.size(); ++i) {
            unsigned int op_code            = str_to_uint(single_qubit_gate_config[i][0]);
            std::string  op_name            = single_qubit_gate_config[i][1];
            unsigned int op_duration        = str_to_uint(single_qubit_gate_config[i][2]);
            opcode_to_opname_lut[op_code]   = op_name;
            single_qubit_gate_time[op_name] = op_duration;
        }
        for (size_t i = 0; i < two_qubit_gate_config.size(); ++i) {
            unsigned int op_code          = str_to_uint(two_qubit_gate_config[i][0]);
            std::string  op_name          = two_qubit_gate_config[i][1];
            unsigned int op_duration      = str_to_uint(two_qubit_gate_config[i][2]);
            opcode_to_opname_lut[op_code] = op_name;
            two_qubit_gate_time[op_name]  = op_duration;
        }
    } else {
        for (size_t i = 0; i < single_qubit_gate_config.size(); ++i) {
            std::string  op_name            = single_qubit_gate_config[i][0];
            unsigned int op_duration        = str_to_uint(single_qubit_gate_config[i][1]);
            single_qubit_gate_time[op_name] = op_duration;
        }
        for (size_t i = 0; i < two_qubit_gate_config.size(); ++i) {
            std::string  op_name         = two_qubit_gate_config[i][0];
            unsigned int op_duration     = str_to_uint(two_qubit_gate_config[i][1]);
            two_qubit_gate_time[op_name] = op_duration;
        }
    }
}

void config_reader::init_cmdparser(int argc, char* argv[]) {

    if (argc < 2) {
        // when running with no parameter, usage should be called
        // usage() function cannot be called outside cmdparser, so add extra -h into argument list
        std::cout << "No valid parameter has been specified." << std::endl;
        argv[argc] = "-h\0";
        argc += 1;
    }

    try {
        cmdparser = new cli::Parser(argc, argv);
    } catch (...) {
        std::cerr << "config_reader: Can't initialize command line parser. Simulation aborts!"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    configure_cmdparser();
}

void config_reader::configure_cmdparser() {

    std::vector<std::string> dump_addr_and_size;
    dump_addr_and_size.push_back("0x0000");
    dump_addr_and_size.push_back("0x10000");

    // configure arguments
    cmdparser->set_optional<std::string>("a", "asm", "",
                                         "Specify assembly file which fed to simulation.");
    cmdparser->set_optional<std::string>("b", "bin", "",
                                         "Specify binary file which fed to simulation.");
    cmdparser->set_optional<std::string>(
      "c", "config", "",
      "Specify configuration file which includes all configs. A typical configuration file is "
      "<CACTUS_root>\\test_files\\test_input_file_list.json.");
    cmdparser->set_optional<std::string>(
      "d", "dm_size", "1M", "Specify data memory size, size unit can be \"M\" or \"K\".");
    cmdparser->set_optional<std::string>(
      "f", "file", "",
      "Specify the name of data memory dump file. Memory will "
      "not dump to a file if file name is not specified by '-f'.");
    cmdparser->set_optional<std::string>(
      "g", "gate_config", "",
      "Specify qubit gate configuration file. A typical configuration file is "
      "<CACTUS_root>\\test_files\\hw_config\\qubit_gate_config.json.");
    cmdparser->set_optional<std::string>(
      "l", "log_level", "",
      "Specify log level configuration file. A configuration config "
      "file is <CACTUS_root>\\test_files\\log_levels.json.");
    cmdparser->set_optional<std::string>("m", "mock_meas", "",
                                         "Specify the file name of mock measurement result.");
    cmdparser->set_optional<unsigned int>("n", "q_num", 7, "Specify qubit number.");
    cmdparser->set_optional<std::string>(
      "o", "output", "./sim_output/",
      "Specify ouput directory for simluation intermediate output.");
    cmdparser->set_optional<unsigned int>(
      "q", "q_sim", 0, "Specify qubit simulator, 0 for Quantumsim and 1 for QIcircuit.");
    cmdparser->set_optional<unsigned int>("r", "run", 3000, "Specify total simulation cycles.");
    cmdparser->set_optional<std::vector<std::string>>(
      "s", "store", dump_addr_and_size,
      "Specify memory start address and size that will be written into file. The base used is "
      "determined by the format. It is used like the one \"-s 0x0000 0x10000\".");
    cmdparser->set_optional<std::string>(
      "t", "tp_config", "",
      "Specify topology configuration file. A typical configuration file is "
      "<CACTUS_root>\\test_files\\hw_config\\cclight_config.json.");
    cmdparser->set_optional<unsigned int>("v", "vliw_width", 2, "Specify VLIW width.");
}

void config_reader::run_cmdparser() {

    cmdparser->run_and_exit_if_error();

    // unsigned int
    num_qubits        = cmdparser->get<unsigned int>("n");
    unsigned int qsim = cmdparser->get<unsigned int>("q");
    num_sim_cycles    = cmdparser->get<unsigned int>("r");
    vliw_width        = cmdparser->get<unsigned int>("v");

    // std::string
    qisa_asm_fn          = cmdparser->get<std::string>("a");
    qisa_bin_fn          = cmdparser->get<std::string>("b");
    config_file_list_fn  = cmdparser->get<std::string>("c");
    data_mem_size_str    = cmdparser->get<std::string>("d");
    data_mem_dump_fn     = cmdparser->get<std::string>("f");
    qubit_gate_config_fn = cmdparser->get<std::string>("g");
    log_level_fn         = cmdparser->get<std::string>("l");
    output_dir           = cmdparser->get<std::string>("o");
    topology_fn          = cmdparser->get<std::string>("t");
    mock_msmt_res_fn     = cmdparser->get<std::string>("m");

    // check whether there is a log_level json file in executable directory
    if (log_level_fn.empty()) {
        std::string path = abs_dir_path_of_exe();
        log_level_fn     = path + "/log_levels.json";
    }

    // create logger
    std::ifstream log_level_fin;
    log_level_fin.open(log_level_fn, std::ios::in);
    if (log_level_fin) {
        log_level_fin.close();
        read_log_levels(log_level_fn);
    } else {
        set_log_level_default();
    }

    auto logger = get_logger_or_exit("console");

    // check -s setting
    auto vec_str = cmdparser->get<std::vector<std::string>>("s");
    if (vec_str.size() != 2) {  // check vector size
        logger->error("config_reader: '-s' parameter is setting incorrectly. Simulation aborts!");
        exit(EXIT_FAILURE);
    }
    try {
        dump_start_addr = std::stoi(vec_str[0], nullptr, 0);
        dump_mem_size   = std::stoi(vec_str[1], nullptr, 0);
    } catch (std::exception& e) {
        logger->error(
          "config_reader: '-s' parameter is setting incorrectly. Exception info: {}. Simulation "
          "aborts!",
          e.what());
        exit(EXIT_FAILURE);
    }

    // set data memory size
    init_data_memory(data_mem_size_str);

    // set memory dump
    data_memory->set_dump(dump_start_addr, dump_mem_size);

    if (!qisa_bin_fn.empty()) {
        instruction_type = Instruction_type::BIN;
    } else {
        instruction_type = Instruction_type::ASM;
    }

    set_qubit_gate_default();

    // set quantum simulator
    switch (qsim) {
        case 0:
            qubit_simulator = Qubit_simulator_type::QUANTUMSIM;
            break;
        case 1:
            qubit_simulator = Qubit_simulator_type::QICIRCUIT;
            logger->error("config_reader: QICircuit is not support currently. Simulation aborts!");
            exit(EXIT_FAILURE);
            break;

        default:
            logger->error(
              "config_reader: The qubit simulator is not configured correctly. Two qubit"
              "simulators are support: 0 for Quantumsim and 1 for QIcircuit. Simulation aborts!");
            exit(EXIT_FAILURE);
            break;
    }

    // read config from configuration files
    read_config(config_file_list_fn);

    if (qisa_bin_fn.empty() && qisa_asm_fn.empty()) {
        logger->error("config_reader: No valid input bin or asm file. Simulation aborts!");
        exit(EXIT_FAILURE);
    }

    delete cmdparser;
    cmdparser = nullptr;
}

void config_reader::init_data_memory(const std::string& data_size) {

    auto logger = get_logger_or_exit("console");

    std::string size_str = data_size;
    trim(size_str);

    unsigned int size_unit = 1;
    unsigned int number;
    std::string  number_str;

    // get size unit
    switch (size_str[size_str.size() - 1]) {
        case 'G':
        case 'g':
            logger->error(
              "config_reader: Simulator cannot allocate data memory with size {}. Simulation "
              "aborts!",
              data_size);
            exit(EXIT_FAILURE);
            break;
        case 'M':
        case 'm':
            size_unit = 1 << 20;
            number_str.assign(size_str.begin(), size_str.end() - 1);
            break;
        case 'K':
        case 'k':
            size_unit = 1 << 10;
            number_str.assign(size_str.begin(), size_str.end() - 1);
            break;
        default:
            logger->error("config_reader: Unrecognized size unit '{}'. Simulation aborts!",
                          data_size);
            exit(EXIT_FAILURE);
            break;
    }
    number = str_to_uint(number_str);

    // initial data memory
    data_memory = new Data_memory(number * size_unit);
}

void config_reader::read_log_levels(std::string log_level_fn) {

    auto logger = safe_create_logger("console");
    logger->set_level(spdlog::level::from_str("error"));

    logger->trace("The logging level config file choosed is '{}'.", log_level_fn);

    json config;
    try {
        config = load_json(log_level_fn);
    } catch (...) {
        logger->error(
          "config_reader: Failed to load the logging level config file : {}. Simulation "
          "aborts!",
          log_level_fn);
        exit(EXIT_FAILURE);
    }

    std::string str_level;
    std::string logger_name;

    logger->trace("Start reading the logging levels...");

    std::shared_ptr<spdlog::logger> new_logger = nullptr;

    try {

        for (auto it = config.begin(); it != config.end(); ++it) {

            logger_name = std::string(it.key());
            read_json_object(config, str_level, logger_name);

            std::vector<std::string> str_log_levels SPDLOG_LEVEL_NAMES;

            if (std::find(str_log_levels.begin(), str_log_levels.end(), str_level) ==
                str_log_levels.end()) {

                std::stringstream ss;
                ss << "config_reader: Undefined logger level ('" << str_level
                   << "') found in the logging level configuration. " << std::endl;

                ss << "    Allowed levels are: " << std::endl;

                for (size_t i = 0; i < str_log_levels.size(); i++) {
                    ss << "        " << str_log_levels[i] << std::endl;
                }
                ss << "Simulation aborts!";

                logger->error("{}", ss.str());
                exit(EXIT_FAILURE);
            }

            new_logger = nullptr;
            new_logger = safe_create_logger(logger_name);
            new_logger->set_level(spdlog::level::from_str(str_level));
            logger->trace("The logging level of the logger {} is: {}", logger_name, str_level);
        }

    } catch (...) {

        logger->error("config_reader: The logging level configuration file is broken. Abort.");
        exit(EXIT_FAILURE);
    }

    logger->trace("Finished reading the logging level configuration.");
}

void config_reader::read_config(std::string config_file_list_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);

    if (!config_file_list_fn.empty()) {
        read_config_file_list(config_file_list_fn);
    }

    if (!topology_fn.empty()) {
        read_from_file(topology_fn);
    }
    // elec_config and mock result is not need in this version
    // read_electronic_config(elec_config_fn);
    // read_mock_msmt_result(msmt_res_fn);
    if (!qubit_gate_config_fn.empty()) {
        read_qubit_gate_config(qubit_gate_config_fn);
    }

    create_dir_if_not_exist(output_dir);

    if (!config_file_list_fn.empty()) {
        logger->trace("Finished reading configuration successfully.");
    }
}

void config_reader::find_config_file(const std::string& config_file_list_fn,
                                     std::string&       config_fn) {
    // If config_fn doesn't exist, try prefixing the dirname of the config file
    // list JSON file; the path may be specified relative to this file.

    FILE* file;
    if (spdlog::details::os::fopen_s(&file, config_fn.c_str(), "r") == 0) {
        // Path is absolute or relative to working directory.
        fclose(file);
        return;
    }

    std::string prefixed_fn = dir_name(config_file_list_fn) + "/" + config_fn;
    if (spdlog::details::os::fopen_s(&file, prefixed_fn.c_str(), "r") == 0) {
        // Path was relative to config file list file, update it.
        config_fn = prefixed_fn;
        fclose(file);
    }
}

void config_reader::read_config_file_list(std::string config_file_list_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);
    logger->trace("Reading the config file list from '{}'.", config_file_list_fn);

    json config;
    try {
        config = load_json(config_file_list_fn);
    } catch (...) {
        logger->error("config_reader: Failed to load the config-file-list file: {}.",
                      config_file_list_fn);
        exit(EXIT_FAILURE);
    }

    try {
        read_json_object(config, topology_fn, "quantum chip topology");
        find_config_file(config_file_list_fn, topology_fn);

        // elec config is not used in current version
        // read_json_object(config, elec_config_fn, "electronic setup");
        // find_config_file(config_file_list_fn, elec_config_fn);

        // control store config is not used in current version
        // read_json_object(config, control_store_fn, "control store");
        // find_config_file(config_file_list_fn, control_store_fn);

        read_json_object(config, qisa_bin_fn, "qisa binary");
        find_config_file(config_file_list_fn, qisa_bin_fn);

        read_json_object(config, qisa_asm_fn, "qisa assemble");
        find_config_file(config_file_list_fn, qisa_asm_fn);

        read_json_object(config, mock_msmt_res_fn, "mock result");
        find_config_file(config_file_list_fn, mock_msmt_res_fn);

        read_json_object(config, qubit_gate_config_fn, "qubit_gate_config");
        find_config_file(config_file_list_fn, qubit_gate_config_fn);

        read_json_object(config, output_dir, "output directory");

        // msmt res file is not used in current version
        // read_json_object(config, msmt_res_fn, "mock measurement results");
        // find_config_file(config_file_list_fn, msmt_res_fn);

        logger->debug("topology_fn: {}", topology_fn);
        // logger->debug("elec_config_fn: {}", elec_config_fn);
        // logger->debug("control_store_fn: {}", control_store_fn);
        logger->debug("qisa_bin_fn: {}", qisa_bin_fn);
        logger->debug("mock_msmt_res_fn: {}", mock_msmt_res_fn);
        logger->debug("output_dir: {}", output_dir);
        // logger->debug("msmt_res_fn: {}", msmt_res_fn);
        logger->debug("qisa_asm_fn: {}", qisa_asm_fn);
        logger->debug("qubit_gate_config_fn: {}", qubit_gate_config_fn);

    } catch (...) {
        logger->error("config_reader: The config-file-list file is broken. Abort.");
        exit(EXIT_FAILURE);
    }
}

void config_reader::read_from_file(std::string config_file_name) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);
    logger->trace("The qubit layout config file chosen is '{}'.", config_file_name);

    json config;
    try {
        config = load_json(config_file_name);
    } catch (...) {
        logger->error("config_reader: Failed to load the config file : {}.", config_file_name);
        exit(EXIT_FAILURE);
    }

    std::stringstream ss;

    json hardware_settings;
    json js_resources;

    json                             js_topology;
    json                             js_edges;
    std::vector<std::pair<int, int>> directed_edges;

    unsigned int num_mw_channels = 0;
    unsigned int quant_sim;
    unsigned int instr_type;

    try {
        read_json_object(config, hardware_settings, "hardware_settings");
        // read_json_object(config, js_resources, "resources");
        read_json_object(config, js_topology, "topology");

        read_json_object(config, num_sim_cycles, "num_sim_cycles");
        logger->debug("Simulator will run {} cycles.", num_sim_cycles);

        read_json_object(config, instr_type, "instruction_type");

        read_json_object(config, quant_sim, "qubit_simulator");

        read_json_object(hardware_settings, num_qubits, "qubit_number");
        logger->debug("There are {} qubits.", num_qubits);

        read_json_object(hardware_settings, vliw_width, "vliw_width");
        logger->debug("There are {} vliw pipelanes.", vliw_width);

        read_json_object(hardware_settings, data_mem_size_str, "data_memory_size");
        logger->debug("Data memory size is {}.", data_mem_size_str);

        /* is not used in this version
        num_flux_devices = num_qubits / num_flux_chnl_per_device + 1;
        logger->debug("There are {} flux AWGs used.", num_flux_devices);

        json js_qwgs;
        read_json_object(js_resources, js_qwgs, "qwgs");
        read_json_object(js_qwgs, num_mw_channels, "count");
        logger->debug("There are {} microwave channels.", num_mw_channels);

        json js_meas_units;
        read_json_object(js_resources, js_meas_units, "meas_units");
        read_json_object(js_meas_units, num_msmt_devices, "count");
        logger->debug("There are {} measurement devices.", num_msmt_devices);
        */

        read_json_object(js_topology, js_edges, "edges");
        num_directed_edges = static_cast<unsigned int>(js_edges.size());
        // read_json_object(js_edges, num_directed_edges, "count");
        logger->debug("The number of directed edges: {}", num_directed_edges);

        directed_edges.resize(num_directed_edges, std::make_pair(-1, -1));

        json connection_map;
        json qubit_pair;

        int id  = -1;
        int src = -1;
        int dst = -1;

        for (size_t i = 0; i < num_directed_edges; ++i) {
            read_json_object(js_edges[i], id, "id");
            read_json_object(js_edges[i], src, "src");
            read_json_object(js_edges[i], dst, "dst");

            if (!check_int_range(id, 0, num_directed_edges)) {
                logger->error(
                  "config_reader: The edge id ({}) is incorrect. Should be within [0, {}].", id,
                  num_directed_edges);
                exit(EXIT_FAILURE);
            }

            if (!check_int_range(src, 0, num_qubits)) {
                logger->error(
                  "config_reader: Source qubit id ({}) is incorrect. Should be within [0, {}].",
                  src, num_qubits);
                exit(EXIT_FAILURE);
            }

            if (!check_int_range(dst, 0, num_qubits)) {
                logger->error(
                  "config_reader: Target qubit id ({}) is incorrect. Should be within [0, {}].",
                  dst, num_qubits);
                exit(EXIT_FAILURE);
            }

            directed_edges[id] = std::make_pair(src, dst);
        }

        logger->debug("The directed edges:");
        for (size_t i = 0; i < directed_edges.size(); i++) {
            logger->debug("\t{}: [{}, {}]", i, directed_edges[i].first, directed_edges[i].second);
        }

        /* hardware setting is not used in this version
        nearby_qubits.clear();
        nearby_qubits.resize(num_qubits);

        for (size_t i = 0; i < directed_edges.size(); i++) {
            src = directed_edges[i].first;
            dst = directed_edges[i].second;

            if (find(nearby_qubits[src].begin(), nearby_qubits[src].end(), dst) ==
                nearby_qubits[src].end()) {

                nearby_qubits[src].push_back(dst);
            }

            if (find(nearby_qubits[dst].begin(), nearby_qubits[dst].end(), src) ==
                nearby_qubits[dst].end()) {

                nearby_qubits[dst].push_back(src);
            }
        }

        ss.str("");
        ss << "The nearby qubits for each qubit are: \n";
        for (size_t i = 0; i < num_qubits; ++i) {
            ss << "    " << i << ": ";
            for (size_t j = 0; j < nearby_qubits[i].size(); ++j) {
                ss << nearby_qubits[i][j] << " ";
            }
            ss << std::endl;
        }

        logger->debug("{}", ss.str());

        connection_map.clear();
        read_json_object(js_qwgs, connection_map, "connection_map");

        if (connection_map.size() != num_mw_channels) {
            logger->error(
              "The number of microwave channels listed ({}) does not match the count ({}).",
              connection_map.size(), num_mw_channels);
            exit(EXIT_FAILURE);
        }

        qubits_in_each_mw_channel.clear();
        qubits_in_each_mw_channel.resize(num_mw_channels);

        json qubits_in_channel;
        for (size_t i = 0; i < num_mw_channels; ++i) {
            // read the JSON object corresponding to the qubit list
            // of the i-th microwave channel
            qubits_in_channel.clear();
            read_json_object(connection_map, qubits_in_channel, std::to_string(i));

            qubits_in_each_mw_channel[i].clear();
            qubits_in_each_mw_channel[i].resize(qubits_in_channel.size());

            // Fetch the qubit list of each channel
            ss.str("");
            for (size_t j = 0; j < qubits_in_channel.size(); ++j) {

                qubits_in_each_mw_channel[i][j] = qubits_in_channel[j];

                ss << qubits_in_each_mw_channel[i][j] << " ";
            }
            logger->debug("Microwave channel {} has the following qubits: [{}]", i, ss.str());
        }

        read_json_object(js_qwgs, num_mw_devices, "num_devices");
        logger->debug("There are {} microwave devices.", num_mw_devices);

        channels_of_mw_device.clear();
        channels_of_mw_device.resize(num_mw_devices);

        json device_channel_map;
        read_json_object(js_qwgs, device_channel_map, "device_channel_map");

        json js_channels_of_mw_device;
        for (size_t i = 0; i < num_mw_devices; ++i) {
            js_channels_of_mw_device.clear();
            read_json_object(device_channel_map, js_channels_of_mw_device, std::to_string(i));

            channels_of_mw_device[i].clear();
            channels_of_mw_device[i].resize(js_channels_of_mw_device.size());

            // Fetch the channel list of each device
            ss.str("");
            for (size_t j = 0; j < js_channels_of_mw_device.size(); ++j) {

                channels_of_mw_device[i][j] = js_channels_of_mw_device[j];

                ss << channels_of_mw_device[i][j] << " ";
            }
            logger->debug("Microwave device {} has the following microwave channels: [{}]", i,
                          ss.str());
        }

        mw_device_array.clear();
        mw_device_array.resize(num_mw_devices);

        unsigned int cur_id = 0;
        // put the mw device information into the data structure
        for (size_t i = 0; i < mw_device_array.size(); ++i) {

            mw_device_array[i].channels.clear();

            auto num_channels_of_mw_device = channels_of_mw_device[i].size();
            mw_device_array[i].channels.resize(num_channels_of_mw_device);

            for (size_t j = 0; j < mw_device_array[i].channels.size(); ++j) {

                mw_device_array[i].channels[j].global_unique_id = cur_id;

                mw_device_array[i].channels[j].target_qubits.clear();
                mw_device_array[i].channels[j].target_qubits = qubits_in_each_mw_channel[cur_id];

                for (std::vector<unsigned int>::iterator it =
                       qubits_in_each_mw_channel[cur_id].begin();
                     it != qubits_in_each_mw_channel[cur_id].end(); it++) {

                    qubit_chnnl_pos[*it] = std::make_pair(i, j);
                }

                cur_id++;
            }
        }

        connection_map.clear();
        read_json_object(js_meas_units, connection_map, "connection_map");

        if (connection_map.size() != num_msmt_devices) {
            logger->error("The number of feedlines listed ({}) does not match the count ({}).",
                          connection_map.size(), num_msmt_devices);
            exit(EXIT_FAILURE);
        }

        qubits_in_each_feedline.clear();
        qubits_in_each_feedline.resize(num_msmt_devices);

        json js_qubits_in_feedline;

        for (size_t i = 0; i < num_msmt_devices; ++i) {
            js_qubits_in_feedline.clear();
            read_json_object(connection_map, js_qubits_in_feedline, std::to_string(i));

            qubits_in_each_feedline[i].clear();
            qubits_in_each_feedline[i].resize(js_qubits_in_feedline.size());

            ss.str("");
            for (size_t j = 0; j < js_qubits_in_feedline.size(); ++j) {
                qubits_in_each_feedline[i][j] = js_qubits_in_feedline[j];
                ss << qubits_in_each_feedline[i][j] << " ";
            }
            logger->debug("The feedline {} connects to the following qubits: [{}] ", i, ss.str());
        }

        // TODO: to add configuration correctness checking.

        json js_frequency;
        read_json_object(config, js_frequency, "frequency");

        if (js_frequency.size() != num_qubits) {
            logger->error(
              "The number of frequencies listed ({}) does not match the number"
              " of qubits ({}).",
              js_frequency.size(), num_qubits);
            exit(EXIT_FAILURE);
        }
        // set other qubit frequency 0.0
        qubit_frequency.clear();
        // default frequency is 0.0
        qubit_frequency.resize(num_qubits, 0.0);
        for (size_t i = 0; i < js_frequency.size(); ++i) {
            read_json_object(js_frequency, qubit_frequency[i], std::to_string(i));
            logger->debug("The frequency of qubit {} is {} GHz.", i, qubit_frequency[i] / 1e9);
        }
        */
    } catch (...) {
        logger->error(
          "config_reader: The topology configuration file is broken. Simulation aborts!");
        exit(EXIT_FAILURE);
    }

    //  set instruction type
    switch (instr_type) {
        case 0:
            instruction_type = Instruction_type::BIN;
            break;
        case 1:
            instruction_type = Instruction_type::ASM;
            break;

        default:
            logger->error(
              "config_reader: The instruction type has not been configured correctly. Two types "
              "are support: 0 for binary and 1 for assembly. Simulation aborts!");
            exit(EXIT_FAILURE);
            break;
    }

    // set quantum simulator
    switch (quant_sim) {
        case 0:
            qubit_simulator = Qubit_simulator_type::QUANTUMSIM;
            break;
        case 1:
            qubit_simulator = Qubit_simulator_type::QICIRCUIT;
            logger->error("config_reader: QICircuit is not support currently. Simulation aborts!");
            exit(EXIT_FAILURE);
            break;

        default:
            logger->error(
              "config_reader: The qubit simulator has not been configured correctly. Two qubit "
              "simulators are support: 0 for quantumsim and 1 for QIcircuit. Simulation aborts!");
            exit(EXIT_FAILURE);
            break;
    }

    in_edges_of_qubit.clear();
    out_edges_of_qubit.clear();
    in_edges_of_qubit.resize(num_directed_edges);
    out_edges_of_qubit.resize(num_directed_edges);
    for (size_t i = 0; i < in_edges_of_qubit.size(); ++i) {
        in_edges_of_qubit[i].clear();
        out_edges_of_qubit[i].clear();
    }

    unsigned int left_qubit  = 0;
    unsigned int right_qubit = 0;

    for (size_t i = 0; i < num_directed_edges; i++) {

        left_qubit  = directed_edges[i].first;
        right_qubit = directed_edges[i].second;

        out_edges_of_qubit[left_qubit].push_back(static_cast<unsigned int>(i));
        in_edges_of_qubit[right_qubit].push_back(static_cast<unsigned int>(i));
    }

    /*
        for (size_t i = 0; i < num_qubits; i++) {
            if (out_edges_of_qubit[i].size() != in_edges_of_qubit[i].size()) {
                logger->error(
                  "Quantum chip topology (layout) specification is not symmetry: qubit {} has {} "
                  "input edges but {} output edges. Simulation aborts!",
                  i, out_edges_of_qubit[i].size(), in_edges_of_qubit[i].size());
            }
        }
    */

    /* is not used in this version
        size_t degree;

        ss.str("");
        for (size_t i = 0; i < num_qubits; ++i) {

            degree = out_edges_of_qubit[i].size();

            for (size_t j = 0; j < degree; ++j) {

                ss << std::hex << out_edges_of_qubit[i][j];

                if (j < (degree - 1)) ss << " | ";
            }

            logger->trace("qubit {} act as left qubit in mask: {}", i, ss.str());
        }

        ss.str("");
        for (size_t i = 0; i < num_qubits; ++i) {

            degree = in_edges_of_qubit[i].size();
            for (size_t j = 0; j < degree; ++j) {

                ss << std::hex << in_edges_of_qubit[i][j];

                if (j < (degree - 1)) ss << " | ";
            }

            logger->trace("qubit {} act as right qubit in mask: {}", i, (ss.str()));
        }
        */

}  // end of read_from_file()

void config_reader::read_qubit_gate_config(std::string config_file_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);

    opcode_to_opname_lut.clear();
    single_qubit_gate_time.clear();
    two_qubit_gate_time.clear();

    json config;
    try {
        config = load_json(config_file_fn);
    } catch (...) {
        logger->error("config_reader: Failed to load the qubit gate config file : {}.",
                      config_file_fn);
        exit(EXIT_FAILURE);
    }

    unsigned int op_code;
    std::string  op_name;
    unsigned int gate_time;
    json         js_gates_of_eqasm;
    json         js_gates_and_time;
    json         js_op_name_and_time;

    // read gates mapping config
    read_json_object(config, js_gates_of_eqasm, "gates_of_eqasm");

    // assembly or binary
    if (instruction_type == Instruction_type::BIN) {
        read_json_object(js_gates_of_eqasm, js_gates_and_time, "binary");
    } else {
        read_json_object(js_gates_of_eqasm, js_gates_and_time, "assembly");
    }

    // read single qubit gate config
    json js_single_qubit_gate_config;
    json js_two_qubit_gate_config;
    read_json_object(js_gates_and_time, js_single_qubit_gate_config, "single_qubit_gate");
    read_json_object(js_gates_and_time, js_two_qubit_gate_config, "two_qubit_gate");

    if (instruction_type == Instruction_type::BIN) {

        for (auto it = js_single_qubit_gate_config.begin(); it != js_single_qubit_gate_config.end();
             ++it) {
            op_code = std::stoi(it.key());
            read_json_object(js_single_qubit_gate_config, js_op_name_and_time, it.key());
            if (js_op_name_and_time.size() < 2) {
                logger->error(
                  "config_reader: Single-qubit gate name and time are not both given at the same "
                  "time. Simulation aborts!");
                exit(EXIT_FAILURE);
            }

            op_name   = js_op_name_and_time[0].get<std::string>();
            gate_time = js_op_name_and_time[1].get<unsigned int>();

            // remove leading and tailing spaces
            trim(op_name);

            // tranform op name to lower case
            transform(op_name.begin(), op_name.end(), op_name.begin(), ::tolower);

            single_qubit_gate_time[op_name] = gate_time;

            opcode_to_opname_lut[op_code] = op_name;
        }

        for (auto it = js_two_qubit_gate_config.begin(); it != js_two_qubit_gate_config.end();
             ++it) {
            op_code = std::stoi(it.key());
            read_json_object(js_two_qubit_gate_config, js_op_name_and_time, it.key());
            if (js_op_name_and_time.size() < 2) {
                logger->error(
                  "config_reader: Two-qubit gate name and time are not both given at the same "
                  "time. "
                  "Simulation aborts!");
                exit(EXIT_FAILURE);
            }

            op_name   = js_op_name_and_time[0].get<std::string>();
            gate_time = js_op_name_and_time[1].get<unsigned int>();

            // remove leading and tailing spaces
            trim(op_name);

            // tranform op name to lower case
            transform(op_name.begin(), op_name.end(), op_name.begin(), ::tolower);

            two_qubit_gate_time[op_name] = gate_time;

            opcode_to_opname_lut[op_code] = op_name;
        }
    } else {
        for (auto it = js_single_qubit_gate_config.begin(); it != js_single_qubit_gate_config.end();
             ++it) {
            op_name = it.key();
            read_json_object(js_single_qubit_gate_config, gate_time, it.key());

            // remove leading and tailing spaces
            trim(op_name);

            // tranform op name to lower case
            transform(op_name.begin(), op_name.end(), op_name.begin(), ::tolower);

            single_qubit_gate_time[op_name] = gate_time;
        }

        for (auto it = js_two_qubit_gate_config.begin(); it != js_two_qubit_gate_config.end();
             ++it) {
            op_name = it.key();
            read_json_object(js_two_qubit_gate_config, gate_time, it.key());

            // remove leading and tailing spaces
            trim(op_name);

            // tranform op name to lower case
            transform(op_name.begin(), op_name.end(), op_name.begin(), ::tolower);

            two_qubit_gate_time[op_name] = gate_time;
        }
    }

    logger->trace("Finished reading qubit gate config successfully.");
}

void config_reader::read_electronic_config(std::string config_file_name) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);
    logger->trace("The electronics config file chosen is '{}'.", config_file_name);

    json config;
    try {
        config = load_json(config_file_name);
    } catch (...) {
        logger->error("config_reader: Failed to load the config file : {}.", config_file_name);
        exit(EXIT_FAILURE);
    }

    try {

        json js_microwave, js_flux;
        read_json_object(config, js_microwave, "microwave");
        read_json_object(config, js_flux, "flux");

        // ---------------------------------------------------------------------------------------
        // reading the microwave AWG lut
        // ---------------------------------------------------------------------------------------
        unsigned int num_mw_codeword = 0;
        read_json_object(js_microwave, num_mw_codeword, "num_codeword");
        logger->trace("The microwave AWG can have at most {} codewords.", num_mw_codeword);

        mw_lut_content.clear();
        mw_lut_content.resize(num_mw_codeword, "");

        json js_mw_lut;
        read_json_object(js_microwave, js_mw_lut, "lut_content");

        int          codeword = 0;
        json         js_op_name_and_time;
        std::string  op_name;
        unsigned int gate_time;

        logger->debug("Start reading the microwave AWG LUT...");
        for (auto it = js_mw_lut.begin(); it != js_mw_lut.end(); ++it) {

            codeword = std::stoi(it.key());

            if (!check_int_range(codeword, 0, num_mw_codeword)) {
                logger->error(
                  "config_reader: Given codeword ({}) is not within the allowed range: [0, {}].",
                  codeword, num_mw_codeword);
                exit(EXIT_FAILURE);
            }

            read_json_object(js_mw_lut, js_op_name_and_time, it.key());

            if (js_op_name_and_time.size() < 2) {
                logger->error(
                  "config_reader: Single-qubit gate name and time are not both given at the same "
                  "time. Simulation aborts!");
                exit(EXIT_FAILURE);
            }

            op_name   = js_op_name_and_time[0].get<std::string>();
            gate_time = js_op_name_and_time[1].get<unsigned int>();

            mw_lut_content[codeword]        = op_name;
            single_qubit_gate_time[op_name] = gate_time;

            logger->debug("\tCodeword {} : {} ({} ns)", codeword, mw_lut_content[codeword],
                          single_qubit_gate_time[op_name]);
        }

        json   js_flux_op_n_delta_freq;
        double delta_freq = 0.0;
        // ---------------------------------------------------------------------------------------
        // reading the flux AWG lut
        // ---------------------------------------------------------------------------------------
        unsigned int num_flux_codeword = 0;
        read_json_object(js_flux, num_flux_codeword, "num_codeword");
        logger->debug("The flux AWG can have at most {} codewords.", num_flux_codeword);

        flux_lut_content.clear();
        flux_lut_content.resize(num_flux_codeword, "");

        json js_flux_lut;
        read_json_object(js_flux, js_flux_lut, "lut_content");

        logger->trace("Start reading the flux AWG LUT...");
        for (auto it = js_flux_lut.begin(); it != js_flux_lut.end(); ++it) {

            codeword = std::stoi(it.key());

            if (!check_int_range(codeword, 0, num_flux_codeword)) {
                logger->error(
                  "config_reader: Given codeword ({}) is not within the allowed range: [0, {}].",
                  codeword, num_flux_codeword);
                exit(EXIT_FAILURE);
            }

            read_json_object(js_flux_lut, js_flux_op_n_delta_freq, it.key());

            if (js_flux_op_n_delta_freq.size() < 2) {
                logger->error(
                  "config_reader: The configuration for flux operation contains less values "
                  "as required (2). Simulation Aborts.");
                exit(EXIT_FAILURE);
            }

            op_name    = js_flux_op_n_delta_freq[0].get<std::string>();
            delta_freq = js_flux_op_n_delta_freq[1].get<double>();

            flux_lut_content[codeword] = op_name;
            flux_delta_freq[op_name]   = delta_freq;

            logger->debug("\tCodeword {} : {} (delta_freq: {} GHz)", codeword,
                          flux_lut_content[codeword], flux_delta_freq[op_name] / 1e9);
        }

        // FIXME: the two-qubit gate time is now hardcoded
        two_qubit_gate_time["CPhase"] = 40;

    } catch (...) {

        logger->error(
          "config_reader: The elctronic configuration file is broken. Simulation abort!");
        exit(EXIT_FAILURE);
    }
}

void config_reader::read_mock_msmt_result(std::string mock_msmt_res_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);

    std::ifstream msmt_res_file;

    msmt_res_file.open(mock_msmt_res_fn, std::ios::in);

    if (!msmt_res_file) {
        logger->error("config_reader: Failed to open file: '{}'. Simulation aborts!",
                      mock_msmt_res_fn);
        exit(EXIT_FAILURE);
    }

    unsigned int current_number = 0;
    msmt_res_array.clear();
    while (msmt_res_file >> current_number) {
        msmt_res_array.push_back(current_number);
    }
    logger->debug("Successfully read the mock measurement results from '{}'.", mock_msmt_res_fn);

    msmt_res_file.close();
}

config_reader::~config_reader() {
    if (data_memory) {
        delete data_memory;
        data_memory = nullptr;
    }
}

}  // end of namespace cactus
