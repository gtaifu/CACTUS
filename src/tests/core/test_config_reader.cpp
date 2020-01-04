#include <exception>
#include <iostream>

#include "global_json.h"
#include "logger_wrapper.h"

using namespace cactus;

int sc_main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <config.json>" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto console = safe_create_logger("console", CODE_POSITION);
    spdlog::set_level(spdlog::level::trace);

    std::cout << "Test_config_reader starts." << std::endl;

    Global_config& global_config = Global_config::get_instance();
    global_config.read_config(argv[1]);

    return 0;
}
