//
// Created by Srf on 2018/10/22.
//

#ifndef CG_LAB2_COMMON_H
#define CG_LAB2_COMMON_H

#include <glm/glm.hpp>
#include <string>
#include <sstream>
#include <vector>

extern void parseByteArray3FromVector3(glm::vec3 color, unsigned char* data);

extern glm::vec4 parseVector4FromString(const std::string &str);

extern glm::vec3 parseVector3FromString(const std::string &str);

extern std::vector<std::string> tokenize(const std::string &string, const std::string &delimiter = " ", bool includeEmpty = false);

extern int toInt(const std::string &str);

extern unsigned int toUInt(const std::string &str);

extern float toFloat(const std::string &str);

#endif //CG_LAB2_COMMON_H
