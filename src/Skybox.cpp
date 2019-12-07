//
// Created by Srf on 2018/11/14.
//

#include "Skybox.h"

Skybox::Skybox (const std::string &environmentMapPath) {
    this->hdrTexture = ResourceManager::loadTexture(
            environmentMapPath.c_str(),
            GL_FALSE,
            "hdrMap",
            Tex2DParams{true, GL_RGB16F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR});
    this->equirectangularToCubemapShader = ResourceManager::loadShader(
            "shader/cubemap_vs.glsl",
            "shader/equirectangular_to_cubemap_fs.glsl",
            nullptr,
            "equirectangularToCubemapShader");
    this->skyboxShader = ResourceManager::loadShader(
            "shader/skybox_vs.glsl",
            "shader/skybox_fs.glsl",
            nullptr,
            "skyboxShader");
    this->environmentCubeMap = ResourceManager::loadCubeMapTexture(
            CubeTexParams{512, GL_RGB16F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR},
            "environmentCubeMap");
    this->cube = new Cube();
}

Skybox::~Skybox() {
    delete cube;
}

CubeMapTexture Skybox::getEnvironmentCubeMap() {
    return this->environmentCubeMap;
}

void Skybox::captureEquirectangularToCubeMap() {
    equirectangularToCubemapShader.use();
    equirectangularToCubemapShader.setInteger("equirectangularMap", 0);
    equirectangularToCubemapShader.setMatrix4("captureProjection", cube->captureProjection);
    glActiveTexture(GL_TEXTURE0);
    this->hdrTexture.bind();

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, cube->captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        equirectangularToCubemapShader.setMatrix4("captureView", cube->equirectangularCaptureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, this->environmentCubeMap.ID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube->RenderCube();
        glBindVertexArray(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->environmentCubeMap.bind();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void Skybox::Init() {
    this->skyboxShader.use();
    this->skyboxShader.setInteger("environmentMap", 0);
    // map equirectangular to cubemap
    this->captureEquirectangularToCubeMap();
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->environmentCubeMap.ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    GLuint vpUniformBlockIndex = glGetUniformBlockIndex(this->skyboxShader.ID, "Matrices");
    glUniformBlockBinding(this->skyboxShader.ID, vpUniformBlockIndex, 0);
}

void Skybox::Draw() {
    this->skyboxShader.use();
    glActiveTexture(GL_TEXTURE0);
    this->environmentCubeMap.bind();
    cube->RenderCube();
    glBindVertexArray(0);
    assert(glGetError() == GL_NO_ERROR);
}

void Skybox::Draw(CubeMapTexture cubeMapTexture) {
    this->skyboxShader.use();
    glActiveTexture(GL_TEXTURE0);
    cubeMapTexture.bind();
    cube->RenderCube();
    glBindVertexArray(0);
    assert(glGetError() == GL_NO_ERROR);
}