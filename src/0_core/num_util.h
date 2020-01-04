#ifndef _NUM_UTIL_H_
#define _NUM_UTIL_H_

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace cactus {

std::string int_2_hex_str(int n, unsigned int hex_width = 8, char fill = '0', bool prefix = true);

void create_dir_if_not_exist(const std::string& dir_name);

std::string extract_integer_words(std::string str);

std::string sep_telf_fn(std::string output_dir, std::string module_name, std::string compact_fn);

std::string full_path_of_exe();
std::string abs_dir_path_of_exe();

std::string dir_name(const std::string& path);

std::vector<std::string> split(const std::string& s, char delimiter);

std::string& trim(std::string& s);

std::string& trim_comments(std::string& s);

int str_to_int(const std::string& s);

unsigned int str_to_uint(const std::string& s);

size_t str_to_sizet(const std::string& s);

int hexstr_to_int(const std::string& s);

unsigned int hexstr_to_uint(const std::string& s);

}  // namespace cactus

#endif  // _NUM_UTIL_H_
