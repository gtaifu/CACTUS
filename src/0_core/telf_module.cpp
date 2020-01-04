#include "telf_module.h"

#include "global_json.h"

namespace cactus {

Telf_module::Telf_module(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n) {

    auto logger = get_logger_or_exit("telf_logger");
}

Telf_module::~Telf_module() {}

void Telf_module::open_telf_file() {
    auto logger = get_logger_or_exit("telf_logger");

    if (is_telf_on) {

        if (telf_fn.length() == 0) {
            logger->error("{}: Required logging file is undefined. Simulation Aborts!");
            exit(EXIT_FAILURE);
        }

        logger->trace("{}: Start opening the telf file: '{}'.", this->name(), telf_fn);

        if (telf_os.is_open()) {
            logger->trace("{}: The file '{}' has been opened.", this->name(), telf_fn);
            return;
        }

        telf_os.open(telf_fn);

        if (telf_os.is_open()) {
            logger->trace("{}: The telf file '{}' has been opened successfully for logging.",
                          this->name(), telf_fn);
        } else {
            logger->error("{}: Failed to open the telf logging file: '{}'. Simulation aborts!",
                          this->name(), telf_fn);
            exit(EXIT_FAILURE);
        }

        add_telf_header();
    }
}

void Telf_module::close_telf_file() {
    auto logger = get_logger_or_exit("telf_logger");

    if (telf_os.is_open()) {
        telf_os.close();
    }

    logger->trace("{}: The telf file '{}' (if open) has been close.", this->name(), telf_fn);
}

}  // end of namespace cactus
