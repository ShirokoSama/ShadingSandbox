//
// Created by Srf on 2019/11/17.
//

#ifndef SANDBOX_BASEPREPASS_H
#define SANDBOX_BASEPREPASS_H

#include "../tools/Shader.h"

class BasePrePass {
public:
    BasePrePass() = default;
    ~BasePrePass() = default;
    virtual void SetShaderUniform(Shader& shader) {}
    virtual void BindShaderUniform() {}
};

#endif //SANDBOX_BASEPREPASS_H
