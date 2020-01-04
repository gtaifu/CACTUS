#include "json_wrapper.h"

namespace cactus {

using json = nlohmann::json;

inline std::shared_ptr<spdlog::logger> safe_create_json_logger() {
    auto logger = safe_create_logger("json_logger");
    logger->set_level(spdlog::level::info);

    return logger;
}

bool check_int_range(int val, int min, int max) {
    auto logger = safe_create_json_logger();

    if (val < min) {
        logger->error("Given value ({}) is smaller than the allowed minimum ({}).", val, min);
        return false;
    }
    if (val > max) {
        logger->error("Given value ({}) is larger than the allowed maximum ({}).", val, max);
        return false;
    }

    return true;
}

json load_json(std::string file_name) {
    auto logger = safe_create_json_logger();

    std::ifstream fs(file_name);

    json j;

    if (fs.is_open()) {
        try {
            fs >> j;
        } catch (json::exception e) {
            logger->error("malformed json file : {}", e.what());
            throw(e);
        }
    } else {
        logger->error("Failed to open file '{}'!", file_name);
        throw(std::string("Failed to open file '") + file_name + std::string("' !"));
    }
    return j;
}

std::string compact_json_dump(json& json_object) {
    std::string str = json_object.dump();
    if (str.length() > 500) {
        str = str.substr(0, 500);
    }
    return str;
}

bool check_key_exist(json& parent, std::string object_name) {

    auto logger = safe_create_json_logger();

    if (parent[object_name].is_null()) {

        logger->error("The key '{}' is not specified in the given json object:\n\t{}", object_name,
                      compact_json_dump(parent));

        throw(std::string("read_json_object() : ") + object_name +
              std::string(" is not specified in the given file!"));

        return false;
    }
    return true;
}

int read_json_object(json& parent, json& child, std::string object_name) {

    auto logger = safe_create_json_logger();

    check_key_exist(parent, object_name);

    child = parent[object_name];

    return 0;
}

int read_json_object(json& parent, std::string& str_value, std::string object_name) {

    auto logger = safe_create_json_logger();

    check_key_exist(parent, object_name);

    if (parent[object_name].type() != json::value_t::string) {

        logger->error(
          "String is expected for the value of the key {}, "
          "but found non-integer in the given file.\n",
          object_name);
    }

    str_value = parent[object_name];

    return 0;
}

int read_json_object(json& parent, int& int_value, std::string object_name) {

    auto logger = safe_create_json_logger();

    check_key_exist(parent, object_name);

    if (parent[object_name].is_number()) {

        int_value = parent[object_name];

    } else {
        logger->error(
          "Integer is expected for the value of the key {}, "
          "but found non-integer in the given file.\n",
          object_name);
    }

    return 0;
}

int read_json_object(json& parent, unsigned int& u_value, std::string object_name) {

    auto logger = safe_create_json_logger();

    check_key_exist(parent, object_name);

    if (parent[object_name].is_number()) {

        u_value = parent[object_name];

    } else {
        logger->error(
          "Integer is expected for the value of the key {}, "
          "but found non-integer in the given file.\n",
          object_name);
    }

    return 0;
}

int read_json_object(json& parent, double& double_value, std::string object_name) {

    auto logger = safe_create_json_logger();

    check_key_exist(parent, object_name);

    if (parent[object_name].is_number()) {

        double_value = parent[object_name];

    } else {
        logger->error(
          "Double is expected for the value of the key {}, "
          "but found non-number in the given file.\n",
          object_name);
    }

    return 0;
}

}  // namespace cactus
