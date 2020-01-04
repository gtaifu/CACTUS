#include "config_setting.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "num_util.h"

using json = nlohmann::json;

namespace cactus {

config_setting::config_setting() {
    std::shared_ptr<spdlog::logger> new_logger = safe_create_logger("config_logger");
    new_logger->set_level(spdlog::level::trace);

    config_file_list_fn =
      "D:\\ihub\\qarchsim\\src\\tests\\assemble_instruction_test\\config_file_list.json";
}

void config_setting::read_config_file_list(std::string config_file_list_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);
    logger->info("Reading the config file list from '{}'.", config_file_list_fn);

    json config;
    try {
        config = load_json(config_file_list_fn);
    } catch (...) {
        logger->error("read_config() :  failed to load the config-file-list file: {}.",
                      config_file_list_fn);
        exit(EXIT_FAILURE);
    }

    try {
        read_json_object(config, opcode_to_opname_fn, "opcode_to_opname");
        find_config_file(config_file_list_fn, opcode_to_opname_fn);

        read_json_object(config, log_level_fn, "logs_level");
        find_config_file(config_file_list_fn, log_level_fn);

        read_json_object(config, global_setting_fn, "global_setting");
        find_config_file(config_file_list_fn, global_setting_fn);

        read_json_object(config, qisa_asm_fn, "assemble_instruction");
        find_config_file(config_file_list_fn, qisa_asm_fn);

        logger->debug("global_setting: {}", qisa_asm_fn);
        logger->debug("opcode_to_opname: {}", opcode_to_opname_fn);
        logger->debug("logs_level: {}", log_level_fn);
        logger->debug("global_setting: {}", global_setting_fn);

    } catch (...) {
        logger->error("The config-file-list file is broken. Abort.");
        exit(EXIT_FAILURE);
    }
}

void config_setting::read_opcode_to_opname_lut(std::string config_file_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);

    json config;
    try {
        config = load_json(config_file_fn);
    } catch (...) {
        logger->error(
          "read_opcode_to_opname_lut() :  failed to load the opcode to opname mapping file : {}.",
          config_file_fn);
        exit(EXIT_FAILURE);
    }

    // read file
    json         js_opcode_to_opname;
    json         js_opname;
    unsigned int opcode;
    std::string  opname;
    read_json_object(config, js_opcode_to_opname, "lut_content");
    for (auto it = js_opcode_to_opname.begin(); it != js_opcode_to_opname.end(); ++it) {
        opcode = std::stoi(it.key());
        read_json_object(js_opcode_to_opname, js_opname, it.key());
        opname = js_opname.get<std::string>();

        opcode_to_opname_lut_content[opcode] = opname;
    }

    logger->info("Finished reading opcode_to_opname mapping successfully.");
}

void config_setting::read_global_setting(std::string config_file_fn) {

    auto logger = get_logger_or_exit("config_logger", CODE_POSITION);

    json         config;
    unsigned int instr_type;
    unsigned int qsim_type;
    try {
        config = load_json(config_file_fn);
    } catch (...) {
        logger->error("read_global_setting() :  failed to load the logging level config file : {}.",
                      config_file_fn);
        exit(EXIT_FAILURE);
    }

    // read file
    read_json_object(config, sim_time, "sim_time");
    read_json_object(config, num_qubits, "num_qubits");
    read_json_object(config, vliw_width, "vliw_width");
    if (vliw_width == 0) {
        logger->error("read_global_setting() :  vliw width sets to 0 in config file: {}.",
                      config_file_fn);
        exit(EXIT_FAILURE);
    }
    read_json_object(config, instr_type, "instruction_type");
    read_json_object(config, qsim_type, "qubit_simulator");

    switch (instr_type) {
        case 0:
            instruction_type = Instruction_type::BIN;
            break;
        case 1:
            instruction_type = Instruction_type::ASM;
            break;

        default:
            logger->error(
              "The instruction type has not been configured correctly. Two types are "
              "support: 0 for bianry and 1 for assemble. Aborts.");
            exit(EXIT_FAILURE);
            break;
    }

    // set quantum simulator
    switch (qsim_type) {
        case 0:
            qubit_simulator = Qubit_simulator_type::QUANTUMSIM;
            break;
        case 1:
            qubit_simulator = Qubit_simulator_type::QICIRCUIT;
            break;

        default:
            logger->error(
              "The qubit simulator has not been configured correctly. Two qubit simulators are "
              "support: 0 for quantumsim and 1 for QIcircuit. Aborts.");
            exit(EXIT_FAILURE);
            break;
    }

    logger->info("Finished reading opcode_to_opname config successfully.");
}

void config_setting::read_log_levels(std::string log_level_fn) {

    auto logger = get_logger_or_exit("console", CODE_POSITION);

    logger->info("The logging level config file chosen is '{}'.", log_level_fn);

    json config;
    try {
        config = load_json(log_level_fn);
    } catch (...) {
        logger->error("read_log_levels() :  failed to load the logging level config file : {}.",
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
                ss << "Undefined logger level ('" << str_level
                   << "') found in the logging level configuration. " << std::endl;

                ss << "    Allowed levels are: " << std::endl;

                for (size_t i = 0; i < str_log_levels.size(); i++) {
                    ss << "        " << str_log_levels[i] << std::endl;
                }
                ss << "Aborts!";

                logger->error("{}", ss.str());
                exit(EXIT_FAILURE);
            }

            new_logger = nullptr;
            new_logger = safe_create_logger(logger_name);
            new_logger->set_level(spdlog::level::from_str(str_level));
            logger->info("The logging level of the logger {} is: {}", logger_name, str_level);
        }

    } catch (...) {

        logger->error("The log-level configuration file is broken. Abort.");
        exit(EXIT_FAILURE);
    }

    logger->info("Finished reading the logging level configuration.");
}

void config_setting::find_config_file(const std::string& config_file_list_fn,
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

}  // namespace cactus
