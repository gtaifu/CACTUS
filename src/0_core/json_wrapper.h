#ifndef _JSON_WRAPPER_H_
#define _JSON_WRAPPER_H_

#include <fstream>
#include <iostream>
#include <string>

#include "json/json.h"
#include "logger_wrapper.h"

namespace cactus {

// check if (val > min) and (val < max)
bool check_int_range(int val, int min, int max);

// load a JSON file as the JSON object
nlohmann::json load_json(std::string file_name);

std::string compact_dump(nlohmann::json& json_object);

int read_json_object(nlohmann::json& parent, nlohmann::json& child, std::string object_name);
int read_json_object(nlohmann::json& parent, std::string& str_value, std::string object_name);
int read_json_object(nlohmann::json& parent, int& int_value, std::string object_name);
int read_json_object(nlohmann::json& parent, unsigned int& u_value, std::string object_name);
int read_json_object(nlohmann::json& parent, double& double_value, std::string object_name);

}  // namespace cactus

#endif  // _JSON_WRAPPER_H_
