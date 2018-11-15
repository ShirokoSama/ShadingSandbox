//
// Created by Srf on 2018/11/14.
//

#ifndef CG_LAB3_SKYBOX_H
#define CG_LAB3_SKYBOX_H

#include <glad/glad.h>
#include <string>
#include <iostream>

#include "../tools/ResourceManager.h"

class Skybox {
public:
    explicit Skybox (const std::string &environmentMapPath) {
        this->hdrTexture = ResourceManager::loadTexture(environmentMapPath.c_str(), GL_FALSE, "hdrMap",
                Tex2DParams{true, GL_RGB16F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR});
        this->equirectangularToCubemapShader = ResourceManager::loadShader(
                "shader/equirectangular_to_cubemap_vs.glsl",
                "shader/equirectangular_to_cubemap_fs.glsl",
                nullptr, "equirectangularToCubemapShader");
        this->skyboxShader = ResourceManager::loadShader("shader/skybox_vs.glsl", "shader/skybox_fs.glsl", nullptr, "skyboxShader");
        this->environmentCubeMap = ResourceManager::loadCubeMapTexture(
                CubeTexParams{512, GL_RGB16F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR}, "environmentCubeMap");

        this->skyboxShader.use();
        this->skyboxShader.setInteger("environmentMap", 0);
        // generate vertices and bind VAO/VBO
        this->initCube();
        // map equirectangular to cubemap
        this->captureEquirectangularToCubeMap();
    }

    ~Skybox() = default;
    void Draw();
    void Clear();
    void BindEnvironmentCubeMap();

    Shader skyboxShader{};
private:
    // generate vertices and bind VAO/VBO
    void initCube();
    void captureEquirectangularToCubeMap();

    GLuint cubeVAO{}, cubeVBO{};
    Texture2D hdrTexture;
    CubeMapTexture environmentCubeMap;
    Shader equirectangularToCubemapShader{};
};


#endif //CG_LAB3_SKYBOX_H
