//
// Created by Srf on 2019/11/17.
//

#ifndef SANDBOX_IBLPREPASS_H
#define SANDBOX_IBLPREPASS_H


#include "BasePrePass.h"
#include "../tools/Cube.h"
#include "../tools/Quad.h"
#include "../tools/ResourceManager.h"

class IblPrePass : public BasePrePass {
public:
    explicit IblPrePass(CubeMapTexture environmentCubeMap);
    ~IblPrePass();
    void DoPrePass();
    void SetShaderUniform(Shader& shader) override;
    void BindShaderUniform() override;

    CubeMapTexture environmentCubeMap;
    CubeMapTexture irradianceCubeMap;
    Shader irradianceShader{};
    CubeMapTexture preFilterCubeMap;
    Shader preFilterShader{};
    Texture2D brdfLUTMap;
    Shader brdfLUTShader{};
    Cube *cube;
    Quad *quad;
};


#endif //SANDBOX_IBLPREPASS_H
