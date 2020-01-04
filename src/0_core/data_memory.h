#ifndef _DATA_MEMORY_H_
#define _DATA_MEMORY_H_

#include <fstream>
#include <iostream>
#include <string>

#include "logger_wrapper.h"

namespace cactus {
// --------------------------------------------------------------------------------------------
// Simulate as a data memory
// --------------------------------------------------------------------------------------------
class Data_memory {
  private:
    unsigned int* memory = nullptr;
    unsigned int  mem_size;
    unsigned int  dump_start;
    unsigned int  dump_size;

  public:
    Data_memory(unsigned int size);
    void         init_data_mem();
    unsigned int read_mem(unsigned int addr);
    void         write_mem(unsigned int addr, unsigned int data);
    void         set_dump(unsigned int start, unsigned int size);
    void         dump(const std::string& file);
    unsigned int get_mem_size();
    ~Data_memory();
};

}  // namespace cactus

#endif  //_DATA_MEMORY_H_