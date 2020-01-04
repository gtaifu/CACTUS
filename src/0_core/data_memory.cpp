#include "data_memory.h"

namespace cactus {

Data_memory::Data_memory(unsigned int size)
    : mem_size(size) {
    memory = new unsigned int[size >> 2];
    init_data_mem();
}

void Data_memory::init_data_mem() {
    // initial memory to 0
    for (unsigned int addr = 0; addr < (mem_size >> 2); ++addr) {
        memory[addr] = 0x0;
    }
}

unsigned int Data_memory::read_mem(unsigned int addr) { return memory[addr]; }

void Data_memory::write_mem(unsigned int addr, unsigned int data) { memory[addr] = data; }

void Data_memory::set_dump(unsigned int start, unsigned int size) {

    auto logger = get_logger_or_exit("console");

    if ((start > mem_size) || ((start + size) > mem_size)) {
        logger->error(
          "data_memory: Memory dump setting is out of data memory size, memory size: '0x{:08x}', "
          "dump start address: '0x{:08x}', dump size: '0x{:08x}'. Simulation aborts!",
          mem_size, start, size);
        exit(EXIT_FAILURE);
    }
    dump_start = start;
    dump_size  = size;
}

void Data_memory::dump(const std::string& file) {

    auto logger = get_logger_or_exit("console");

    std::ofstream f_out(file, std::ios::binary);
    if (!f_out.is_open()) {
        logger->error("data_memory: Failed to open memory dump file '{}'. Simulation aborts!",
                      file);
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = dump_start; i < dump_start + dump_size; i = i + 4) {
        char c0 = memory[i >> 2] & 0xff;
        char c1 = (memory[i >> 2] >> 8) & 0xff;
        char c2 = (memory[i >> 2] >> 16) & 0xff;
        char c3 = (memory[i >> 2] >> 24) & 0xff;
        f_out.put(c0);
        f_out.put(c1);
        f_out.put(c2);
        f_out.put(c3);
    }

    f_out.close();
}

unsigned int Data_memory::get_mem_size() { return mem_size; }

Data_memory::~Data_memory() {
    if (memory) {
        delete memory;
        memory = nullptr;
    }
}

}  // namespace cactus