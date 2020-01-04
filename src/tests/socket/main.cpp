#include <iostream>
#include <systemc>

#include "global_json.h"
#include "qvm_server.h"
#include "socket.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using cactus::config_reader;

using namespace cactus;

int main(int argc, char* argv[]) {

    std::cout << "hello." << std::endl;

    auto telf_logger = safe_create_logger("telf_logger", CODE_POSITION);  // used for telf_module

    auto console_logger =
      safe_create_logger("console", CODE_POSITION);  // used for terminal console

    spdlog::set_level(spdlog::level::trace);  // Set global log level to info

    std::cout << "Main: start initializeing QVM server." << std::endl;
    QVM_server qvm_server;
    qvm_server.launch();
    std::cout << "Main: finished initializeing QVM server." << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
