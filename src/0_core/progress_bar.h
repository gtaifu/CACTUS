/*progress_bar.h
This file is used to show progress bar
*/

#ifndef _PROGRESS_BAR_H_
#define _PROGRESS_BAR_H_

#include <iostream>

namespace cactus {

class Progress_bar {
  public:
    const unsigned int total_cycles;
    const unsigned int bar_width;
    const char         complete_char   = '=';
    const char         incomplete_char = ' ';

  public:
    Progress_bar(unsigned int total, unsigned int width, char complete = '=', char incomplete = ' ')
        : total_cycles(total)
        , bar_width(width)
        , complete_char(complete)
        , incomplete_char(incomplete) {}

    void display_bar(unsigned int cur_cycle) const {

        unsigned int percent = static_cast<unsigned int>(cur_cycle * 100 / total_cycles);
        unsigned int pos     = static_cast<unsigned int>(cur_cycle * bar_width / total_cycles);

        std::cout << "[";
        for (unsigned int i = 0; i < bar_width; ++i) {
            if (i < pos)
                std::cout << complete_char;
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << incomplete_char;
        }
        std::cout << "] "
                  << "%" << percent << "\r";
        std::cout.flush();
    }
};

}  // namespace cactus

#endif  //_PROGRESS_BAR_H_
