//
// Created by Srf on 2019/11/17.
//

#include "IblPrePass.h"
#include <iostream>

IblPrePass::IblPrePass(CubeMapTexture environmentCubeMap) {
    this->environmentCubeMap = environmentCubeMap;
    this->irradianceCubeMap = ResourceManager::loadCubeMapTexture(
            CubeTexParams{32, GL_RGB16F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR},
            "irradianceCubeMap");
    this->irradianceShader = ResourceManager::loadShader(
            "shader/cubemap_vs.glsl",
            "shader/irradiance_convolution.glsl",
            nullptr,
            "irradianceShader");
    this->preFilterCubeMap = ResourceManager::loadCubeMapTexture(
            CubeTexParams{128, GL_RGB16F, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR},
            "preFilterCubeMap");
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    this->preFilterShader = ResourceManager::loadShader(
            "shader/cubemap_vs.glsl",
            "shader/pre_filter.glsl",
            nullptr,
            "preFilterShader");
    this->brdfLUTMap = Texture2D(
            Tex2DParams{true, GL_RG16F, GL_RG, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR});
    this->brdfLUTMap.generate(512, 512, nullptr);
    this->brdfLUTShader = ResourceManager::loadShader(
            "shader/brdf_vs.glsl",
            "shader/brdf_fs.glsl",
            nullptr,
            "brdfLUTShader");
    cube = new Cube();
    quad = new Quad();
}

IblPrePass::~IblPrePass() {
    delete cube;
}

void IblPrePass::DoPrePass() {
    // 1. irradiance
    glBindFramebuffer(GL_FRAMEBUFFER, cube->captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, cube->captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    irradianceShader.use();
    irradianceShader.setInteger("environmentMap", 0);
    irradianceShader.setMatrix4("captureProjection", cube->captureProjection);
    glActiveTexture(GL_TEXTURE0);
    this->environmentCubeMap.bind();

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, cube->captureFBO);
    for (unsigned int i = 0; i < 6; i++) {
        irradianceShader.setMatrix4("captureView", cube->captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, this->irradianceCubeMap.ID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube->RenderCube();
        glBindVertexArray(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. pre_filter
    preFilterShader.use();
    preFilterShader.setInteger("environmentMap", 0);
    preFilterShader.setMatrix4("captureProjection", cube->captureProjection);
    glActiveTexture(GL_TEXTURE0);
    this->environmentCubeMap.bind();

    glBindFramebuffer(GL_FRAMEBUFFER, cube->captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; mip++) {
        unsigned int mipWidth  = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, cube->captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        preFilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; i++) {
            preFilterShader.setMatrix4("captureView", cube->captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preFilterCubeMap.ID, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube->RenderCube();
            glBindVertexArray(0);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. brdf look up table
    glBindFramebuffer(GL_FRAMEBUFFER, cube->captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, cube->captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTMap.ID, 0);

    glViewport(0, 0, 512, 512);
    brdfLUTShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quad->RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void IblPrePass::SetShaderUniform(Shader &shader) {
    shader.use();
    shader.setInteger("environmentMap", 1);
    shader.setInteger("irradianceMap", 2);
    shader.setInteger("preFilterMap", 3);
    shader.setInteger("brdfLUT", 4);
}

void IblPrePass::BindShaderUniform() {
    glActiveTexture(GL_TEXTURE1);
    this->environmentCubeMap.bind();
    glActiveTexture(GL_TEXTURE2);
    this->irradianceCubeMap.bind();
    glActiveTexture(GL_TEXTURE3);
    this->preFilterCubeMap.bind();
    glActiveTexture(GL_TEXTURE4);
    this->brdfLUTMap.bind();
}
