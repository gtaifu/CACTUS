#include "num_util.h"

#include <cstring>
#include <iostream>
#include <vector>

#ifdef WIN32
#include <Shlwapi.h>
#include <windows.h>

#pragma comment(lib, "shlwapi.lib")

#else
#include <libgen.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace cactus {

// TODO: this function is not robust enough. it should be updated.
std::string int_2_hex_str(int n, unsigned int hex_width, char fill, bool prefix) {
    std::stringstream ss;
    ss.str("");

    if (prefix) {
        ss << "0x";
    }

    ss << std::setw(hex_width) << std::setfill(fill) << std::hex << n;

    return ss.str();
}

// =============================================================================
//  remove leading and trailing spaces
// =============================================================================
std::string& trim(std::string& s) {
    if (s.empty()) {
        return s;
    }

    s.erase(0, s.find_first_not_of(" \t"));
    s.erase(s.find_last_not_of(" \t") + 1);

    return s;
}

// =============================================================================
//  remove comments which indicate by first char '#'
// =============================================================================
std::string& trim_comments(std::string& s) {
    if (s.empty()) {
        return s;
    }

    if (s.find_first_of("#") != s.npos) {
        s.erase(s.find_first_of("#"));
    }

    return s;
}

// =============================================================================
//  convert string to int
// =============================================================================
int str_to_int(const std::string& s) {
    if (s.empty()) {
        std::cerr << "Failed to convert empty string to unsigned int. exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    int               i;
    std::stringstream ss;
    ss << s;
    ss >> i;

    return i;
}

// =============================================================================
//  convert string to unsigned int
// =============================================================================
unsigned int str_to_uint(const std::string& s) {
    if (s.empty()) {
        std::cerr << "Failed to convert empty string to unsigned int. exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned int      u;
    std::stringstream ss;
    ss << s;
    ss >> u;

    return u;
}

// =============================================================================
//  convert hex string to int
// =============================================================================
int hexstr_to_int(const std::string& s) {
    if (s.empty()) {
        std::cerr << "Failed to convert empty string to unsigned int. exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    bool sign = (s.find("-") != s.npos);

    std::string hexstr;

    if (s.find("0x") != s.npos) {
        hexstr = s.substr(s.find("0x") + 2);
    } else if (s.find("0X") != s.npos) {
        hexstr = s.substr(s.find("0X") + 2);
    } else {
        std::cerr << "Failed to convert hex string to int: " << s << std::endl;
        exit(EXIT_FAILURE);
    }

    int  result = 0;
    char c;
    for (size_t i = 0; i < hexstr.size(); ++i) {
        c = hexstr[i];
        if (c >= 65 && c <= 70) {
            c = c - 65 + 10;  // ASCII: 'A'->65
        } else if (c >= 97 && c <= 102) {
            c = c - 97 + 10;  // ASCII: 'a'->97
        } else {
            c = c - 48;  // ASCII: '0'->48
        }

        result += (c << (4 * (hexstr.size() - 1 - i)));
    }

    if (sign) {
        return 0 - result;
    } else {
        return result;
    }
}

// =============================================================================
//  convert hex string to unsigned int
// =============================================================================
unsigned int hexstr_to_uint(const std::string& s) {
    if (s.empty()) {
        std::cerr << "Failed to convert empty string to unsigned int. exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string hexstr;

    if (s.find("0x") != s.npos) {
        hexstr = s.substr(s.find("0x") + 2);
    } else if (s.find("0X") != s.npos) {
        hexstr = s.substr(s.find("0X") + 2);
    } else {
        std::cerr << "Failed to convert hex string to int: " << s << ". exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    unsigned int result = 0;
    char         c;
    for (size_t i = 0; i < hexstr.size(); ++i) {
        c = hexstr[i];
        if (c >= 65 && c <= 70) {
            c = c - 65 + 10;
        } else if (c >= 97 && c <= 102) {
            c = c - 97 + 10;  // ASCII: 'a'->97
        } else {
            c = c - 48;  // ASCII: '0'->48
        }

        result += (c << (4 * (hexstr.size() - 1 - i)));
    }

    return result;
}

// =============================================================================
//  convert string to size_t
// =============================================================================
size_t str_to_sizet(const std::string& s) {
    if (s.empty()) {
        std::cerr << "Failed to convert empty string to size_t type. exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t            u;
    std::stringstream ss;
    ss << s;
    ss >> u;

    return u;
}

// =============================================================================
//  Split string with delimiter
// =============================================================================
std::vector<std::string> split(const std::string& s, char delimiter) {

    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       tokenStream(s);

    while (std::getline(tokenStream, token, delimiter)) {

        tokens.push_back(token);
    }

    return tokens;
}

// =============================================================================
// check if directory exists
// TODO: recursively creating the directory.
// =============================================================================

#ifdef WIN32
// ------------------------------------------------------------------
//  Windows
// ------------------------------------------------------------------
void create_dir_if_not_exist(const std::string& dir_name) {

    if (CreateDirectory(dir_name.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) return;

    // Exit when error happens while creating the directory.
    std::cerr << "Failed to create directory '" << dir_name << "'. Abrots!" << std::endl;
    exit(EXIT_FAILURE);
}

#else
// ------------------------------------------------------------------
//  Non-Windows
// ------------------------------------------------------------------
bool check_dir_exist(const std::string& dir_name) {

    struct stat info;

    if (stat(dir_name.c_str(), &info)) {
        return false;
    }

    if (info.st_mode & S_IFDIR) {
        return true;
    }

    return false;
}

void create_dir_if_not_exist(const std::string& dir_name) {

    if (!check_dir_exist(dir_name)) {
        if (mkdir(dir_name.c_str(), 0755)) {
            perror("error creating directory");
            exit(EXIT_FAILURE);
        }
    }
}

#endif

// =============================================================================
// check if directory exists
// TODO: recursively creating the directory.
// =============================================================================

#ifdef WIN32
// ------------------------------------------------------------------
//  Windows
// ------------------------------------------------------------------

std::string TCHAR_to_string(TCHAR* text) {

#ifdef UNICODE

    std::vector<char> buffer;
    int               size = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);

    if (size > 0) {
        buffer.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, text, -1, static_cast<BYTE*>(&buffer[0]), buffer.size(),
                            NULL, NULL);
    } else {
        std::cerr << "Failed to convert TCHAR array into std::string. exit." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string string(&buffer[0]);

#else

    std::string string(text);

#endif

    return string;
}

std::string full_path_of_exe() {
    TCHAR dest[MAX_PATH];
    DWORD length = GetModuleFileName(NULL, dest, MAX_PATH);

    if (length == 0) {

        std::cerr << "Failed to get the path of this executable. exits." << std::endl;
        exit(EXIT_FAILURE);
    }

    return TCHAR_to_string(dest);
}

std::string abs_dir_path_of_exe() {
    TCHAR dest[MAX_PATH];
    DWORD length = GetModuleFileName(NULL, dest, MAX_PATH);

    if (length == 0) {

        std::cerr << "abs_dir_path_of_exe(): Failed to get the path of this executable. exits."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    PathRemoveFileSpecA(dest);  // remove the filename from the path

    return TCHAR_to_string(dest);
}

std::string dir_name(const std::string& path) {
    TCHAR dir[MAX_PATH + 1];
    strncpy_s(dir, path.c_str(), MAX_PATH);
    dir[MAX_PATH] = 0;
    PathRemoveFileSpecA(dir);
    return TCHAR_to_string(dir);
}

#else
// ------------------------------------------------------------------
//  Non-Windows
// ------------------------------------------------------------------
std::string full_path_of_exe() {
    char    path[PATH_MAX + 1];
    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
    if ((len < 0) || (len > PATH_MAX)) {
        std::cerr << "Failed to get the path of this executable. exits." << std::endl;
        exit(EXIT_FAILURE);
    }
    path[(size_t) len] = 0;
    return std::string(path);
}

std::string abs_dir_path_of_exe() {
    char    path[PATH_MAX + 1];
    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
    if ((len < 0) || (len > PATH_MAX)) {
        std::cerr << "Failed to get the path of this executable. exits." << std::endl;
        exit(EXIT_FAILURE);
    }
    path[(size_t) len] = 0;
    return std::string(dirname(path));
}

std::string dir_name(const std::string& path) {
    char dir[PATH_MAX + 1];
    strncpy(dir, path.c_str(), PATH_MAX);
    dir[PATH_MAX] = 0;
    return std::string(dirname(dir));
}

#endif

// Since a single module can be used for multiple times in the same project, we need to use a
//   unique filename for each telf file. This is achieved by appending the index of the modules
//   in the hieriarchy (which is stored in the module_name) to the compact_fn
std::string sep_telf_fn(std::string output_dir, std::string module_name, std::string compact_fn) {

    auto num_str = extract_integer_words(module_name);

    if (output_dir.back() != '/') {
        output_dir = output_dir + "/";
    }

    std::string sep_telf_fn;

    if (num_str.length() > 0)
        sep_telf_fn = output_dir + compact_fn + "_" + num_str + ".csv";
    else
        sep_telf_fn = output_dir + compact_fn + ".csv";

    return sep_telf_fn;
}

// extract all numbers from a string, and combine these numbers into a new string
//   with underscore (_) separating all numbers
bool is_digit(char c) { return (c >= '0' && c <= '9'); }

std::string extract_integer_words(std::string str) {
    std::string num_str;

    size_t pos = 0;
    size_t len = 0;
    while (pos < str.length()) {

        if (is_digit(str[pos])) {

            len = 1;

            while (is_digit(str[pos + len])) {
                len++;
            }

            if (num_str.length() == 0)
                num_str = str.substr(pos, len);
            else
                num_str = num_str + "_" + str.substr(pos, len);

            pos = pos + len + 1;

        } else {
            pos++;
        }
    }

    return num_str;
}

}  // namespace cactus
