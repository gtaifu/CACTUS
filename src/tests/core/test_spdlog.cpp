#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

int main(int argc, char const *argv[])
{
    auto logger = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::trace); // Set global log level to info
    logger->set_pattern("[%T] [%^%l%$] %v");

    auto new_logger = spdlog::get("no_logger");
    if (new_logger == nullptr) {
        std::cout << "empty logger found!" << std::endl;
    } else {
        std::cout << "the pointer is not equal to nullptr: " << new_logger << std::endl;
    }
    return 0;
}
