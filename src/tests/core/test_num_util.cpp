#include <iostream>

#include "num_util.h"

using namespace std;
using namespace cactus;

int test_int_2_hex_str() {
    int num = 15;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num) << endl;
    num = -1;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num) << endl;
    num = 125;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num, 5, ' ', true)
         << endl;
    num = -125;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num, 5, ' ') << endl;
    num = -1;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num) << endl;
    num = -1;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num) << endl;
    num = -1;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num) << endl;
    num = -1;
    cout << "Dec num: " << num << ", hex representation: " << int_2_hex_str(num) << endl;

    return 0;
}

int test_sep_telf_fn() {
    cout << sep_telf_fn("hello/", "module_name", "dummy") << endl;
    cout << sep_telf_fn("hello/world", "module_name", "dummy") << endl;
    cout << sep_telf_fn("hello/world", "module_name12", "dummy") << endl;
    cout << sep_telf_fn("hello/world", "module_1name2", "dummy") << endl;

    return 0;
}

int main(int argc, char const* argv[]) {
    // test_int_2_hex_str();
    test_sep_telf_fn();

    cout << "abs_dir_path_of_exe: " << abs_dir_path_of_exe() << endl;
    cout << "full_path_of_exe: " << full_path_of_exe() << endl;
    return 0;
}
