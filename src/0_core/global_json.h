#ifndef _GLOBAL_JSON_H_
#define _GLOBAL_JSON_H_

#include "config_reader.h"

namespace cactus {

class Global_config : public config_reader {
  public:
    // delete the copy constructor
    Global_config(const Global_config&) = delete;

    // delete the assignment operator
    Global_config& operator=(const Global_config&) = delete;

    static Global_config& get_instance() {
        // call the priviate constructor to initialize a Global_config
        // the static one ensures only one copy of the Global_config
        static Global_config s_instance;
        return s_instance;
    }

  private:
    // priviate constructure, ensure cannot be initialized publicly.
    Global_config()
        : config_reader(){};

    ~Global_config() = default;
};

}  // end of namespace cactus

#endif  // _GLOBAL_JSON_H_
