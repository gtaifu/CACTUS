#include <iostream>

#include "global_json.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using cactus::config_reader;

int main(int argc, char* argv[]) {

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::info);  // Set global log level to info
    console->set_pattern("[%T] [%^%l%$] %v");

    config_reader reader;

    std::string layout_filename("D:/Projects/QuMA_Sim/input_files/cclight_config.json");
    std::string elec_config_filename(
      "D:/Projects/QuMA_Sim/input_files/electronics_content_gb.json");

    reader.read_from_file(layout_filename);
    reader.read_electronic_config(elec_config_filename);

    // reader->write_output();

    system("pause");
    return 0;
}
