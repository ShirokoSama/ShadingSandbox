//
// Created by Srf on 2018/11/14.
//

#ifndef CG_LAB3_SKYBOX_H
#define CG_LAB3_SKYBOX_H

#include <glad/glad.h>
#include <string>
#include <iostream>

#include "../tools/ResourceManager.h"
#include "../tools/Cube.h"

class Skybox {
public:
    explicit Skybox (const std::string &environmentMapPath);
    ~Skybox();
    void Init();
    void Draw();
    void Draw(CubeMapTexture cubeMapTexture);
    CubeMapTexture getEnvironmentCubeMap();

private:
    void captureEquirectangularToCubeMap();

    Texture2D hdrTexture;
    CubeMapTexture environmentCubeMap;
    Shader skyboxShader{};
    Shader equirectangularToCubemapShader{};
    Cube *cube;
};


#endif //CG_LAB3_SKYBOX_H
