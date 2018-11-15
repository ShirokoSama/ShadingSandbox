//
// Created by Srf on 2018/10/21.
//

#include "common.h"
#include <iostream>

void parseByteArray3FromVector3(glm::vec3 color, unsigned char* data) {
    data[0] = static_cast<unsigned char>(255 * color.r);
    data[1] = static_cast<unsigned char>(255 * color.g);
    data[2] = static_cast<unsigned char>(255 * color.b);
}

glm::vec4 parseVector4FromString(const std::string &str) {
    std::istringstream ss(str);
    float x, y, z, w;
    ss >> x >> y >> z >> w;
    return glm::vec4(x, y, z, w);
}

glm::vec3 parseVector3FromString(const std::string &str) {
    std::istringstream ss(str);
    float x, y, z;
    ss >> x >> y >> z;
    return glm::vec3(x, y, z);
}

std::vector<std::string> tokenize(const std::string &string, const std::string &delimiter, bool includeEmpty) {
    std::string::size_type lastPos = 0, pos = string.find_first_of(delimiter, lastPos);
    std::vector<std::string> tokens;
    while (lastPos != std::string::npos) {
        if (pos != lastPos || includeEmpty)
            tokens.push_back(string.substr(lastPos, pos - lastPos));
        lastPos = pos;
        if (lastPos != std::string::npos) {
            lastPos += 1;
            pos = string.find_first_of(delimiter, lastPos);
        }
    }
    return tokens;
}

int toInt(const std::string &str) {
    char *end_ptr = nullptr;
    int result = (int) strtol(str.c_str(), &end_ptr, 10);
    if (*end_ptr != '\0')
        throw "Could not parse integer value \"" + str + "\"";
    return result;
}

unsigned int toUInt(const std::string &str) {
    char *end_ptr = nullptr;
    unsigned int result = (int) strtoul(str.c_str(), &end_ptr, 10);
    if (*end_ptr != '\0')
        throw "Could not parse unsigned integer value \"" + str + "\"";
    return result;
}

float toFloat(const std::string &str) {
    char *end_ptr = nullptr;
    float result = strtof(str.c_str(), &end_ptr);
    if (*end_ptr != '\0')
        throw "Could not parse unsigned float point value \"" + str + "\"";
    return result;
}