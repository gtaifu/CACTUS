//
// Created by leon on 24-3-17.
//

#ifndef _CLOCKED_MODULE_H_
#define _CLOCKED_MODULE_H_

#include <ostream>
#include <string>
#include <systemc>

#include "global_json.h"
#include "logger_wrapper.h"

namespace cactus {

// --------------------------------------------------------------------------------------------
// A base class that provides methods used for telf logging.
// --------------------------------------------------------------------------------------------
class Telf_module : public sc_core::sc_module {
  public:
    Telf_module(const sc_core::sc_module_name& n);

    ~Telf_module();

  public:  // public configurations
    bool        is_telf_on = false;
    std::string telf_fn;

  protected:
    std::ofstream telf_os;

  public:  // methods for telf logging
    void open_telf_file();

    void close_telf_file();

  protected:
    virtual void add_telf_header() {}
    virtual void add_telf_line() {}

  public:
    SC_HAS_PROCESS(Telf_module);
};

}  // namespace cactus

#endif  //_CLOCKED_MODULE_H_
