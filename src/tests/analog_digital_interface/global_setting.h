#ifndef _GLOBAL_SETTING_H_
#define _GLOBAL_SETTING_H_

#include "config_setting.h"

namespace cactus {

class Global_setting : public config_setting {
  public:
    // delete the copy constructor
    Global_setting(const Global_setting&) = delete;

    // delete the assignment operator
    Global_setting& operator=(const Global_setting&) = delete;

    static Global_setting& get_instance() {
        // call the priviate constructor to initialize a Global_config
        // the static one ensures only one copy of the Global_config
        static Global_setting s_instance;
        return s_instance;
    }

  private:
    // priviate constructure, ensure cannot be initialized publicly.
    Global_setting()
        : config_setting(){};

    ~Global_setting() = default;
};

}  // end of namespace cactus

#endif  // _GLOBAL_SETTING_H_
