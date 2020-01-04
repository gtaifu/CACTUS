#ifndef _LOGGER_WRAPPER_H_
#define _LOGGER_WRAPPER_H_

#include <iostream>
#include <sstream>
#include <string>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace cactus {

#define CODE_POSITION (std::string(__FILE__) + ":" + std::to_string(__LINE__))

inline std::shared_ptr<spdlog::logger> get_logger_or_exit(
  const std::string& name, const std::string& pos_info = std::string("")) {

    auto target_logger = spdlog::get(name);

    if (target_logger == nullptr) {

        if (pos_info.length() > 0)
            std::cerr << "Error! " << pos_info << ": logger " << name << " is not found!"
                      << std::endl;
        else
            std::cerr << "Error! Logger " << name << " is not found! Aborts." << std::endl;

        exit(EXIT_FAILURE);
    }

    return target_logger;
}

inline std::shared_ptr<spdlog::logger> safe_create_logger(
  const std::string& name, const std::string& pos_info = std::string("")) {

    auto target_logger = spdlog::get(name);

    if (target_logger == nullptr) {

        target_logger = spdlog::stdout_color_mt(name);

        if (target_logger == nullptr) {

            if (pos_info.length() > 0) {
                std::cerr << "Error! " << pos_info << ": Failed to create the logger '" << name
                          << "'." << std::endl;
            } else {
                std::cerr << "Error!  Failed to create the logger '" << name << "'. Aborts"
                          << std::endl;
            }

            exit(EXIT_FAILURE);
        }

        target_logger->set_pattern("[%T] [%n] %^[%L]%$ %v");
        target_logger->trace("Successfully created this logger.");
    }

    return target_logger;
}

}  // namespace cactus

#endif  // _LOGGER_WRAPPER_H_
