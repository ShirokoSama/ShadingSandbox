//
// Created by Srf on 2017/10/5.
//

#include "ResourceManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include "stb_image.h"
#include "common.h"

std::unordered_map<std::string, Texture2D> ResourceManager::textures;
std::unordered_map<std::string, Shader> ResourceManager::shaders;
std::unordered_map<std::string, MerlBrdfTexture> ResourceManager::merlBrdfs;
std::unordered_map<std::string, CubeMapTexture> ResourceManager::cubeMaps;

Shader ResourceManager::loadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name) {
    shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return shaders[name];
}

Shader ResourceManager::getShader(std::string name) {
    return shaders[name];
}

Texture2D ResourceManager::loadTexture(const GLchar *file, GLboolean alpha, std::string name, Tex2DParams params) {
    textures[name] = loadTextureFromFile(file, alpha, params);
    return textures[name];
}

Texture2D ResourceManager::loadSolidTexture(glm::vec3 color, std::string name) {
    textures[name] = generateSolidTexture(color);
    return textures[name];
}

MerlBrdfTexture ResourceManager::loadMerlBrdfTexture(const GLchar *file, std::string name) {
    merlBrdfs[name] = loadMerlBrdfFromFile(file);
    return merlBrdfs[name];
}

CubeMapTexture ResourceManager::loadCubeMapTexture(CubeTexParams params, std::string name) {
    cubeMaps[name] = generateCubeMapTexture(params);
    return cubeMaps[name];
}

Texture2D ResourceManager::getTexture(std::string name) {
    return textures[name];
}

void ResourceManager::clear() {
    for (auto iter : shaders)
        glDeleteProgram(iter.second.ID);
    for (auto iter : textures)
        glDeleteTextures(1, &iter.second.ID);
    for (auto iter : merlBrdfs)
        glDeleteTextures(1, &iter.second.ID);
    for (auto iter: cubeMaps)
        glDeleteTextures(1, &iter.second.ID);
}

Shader ResourceManager::loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile) {
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        // Open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // If geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception &e) {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const GLchar *vShaderCode = vertexCode.c_str();
    const GLchar *fShaderCode = fragmentCode.c_str();
    const GLchar *gShaderCode = geometryCode.c_str();
    // 2. Now create shader object from source code
    Shader shader = Shader();
    shader.compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const GLchar *file, GLboolean alpha, Tex2DParams params) {
    Texture2D texture = Texture2D();
    if (params.use)
        texture = Texture2D(params);
    if (alpha) {
        texture.internal_format = GL_RGBA;
        texture.image_format = GL_RGBA;
    }
    int width, height, nrChannels;
    std::cout << "Loading texture \"" + std::string(file) + "\" .. ";
    GLvoid *image;
    if (params.data_format == GL_FLOAT)
        image = stbi_loadf(file, &width, &height, &nrChannels, 0);
    else
        image = stbi_load(file, &width, &height, &nrChannels, 0);
    if (image == nullptr)
        throw "Loading texture fail";
    if (nrChannels == 4) {
        texture.internal_format = GL_RGBA;
        texture.image_format = GL_RGBA;
    }
    texture.generate((GLuint)width, (GLuint)height, image);
    stbi_image_free(image);
    std::cout << " done." << std::endl;
    return texture;
}

Texture2D ResourceManager::generateSolidTexture(glm::vec3 color) {
    Texture2D texture = Texture2D();
    unsigned char colorData[3];
    parseByteArray3FromVector3(color, colorData);
    texture.generate(1, 1, colorData);
    return texture;
}

MerlBrdfTexture ResourceManager::loadMerlBrdfFromFile(const GLchar *file) {
    std::cout << "Loading merl brdf file \"" + std::string(file) + "\" .. ";
    // read data from file
    FILE *f = nullptr;
    errno_t err;
    if ((err = fopen_s(&f, file, "rb")) != 0)
        throw "Open merl brdf file fails.";
    int dimension[3];
    fread(dimension, sizeof(int), 3, f);
    int n = dimension[0] * dimension[1] * dimension[2];
    auto brdfData = (double*) malloc(sizeof(double) * 3 * n);
    fread(brdfData, sizeof(double), static_cast<size_t>(3 * n), f);
    fclose(f);

    // scale RGB and format data into RGB 1D-Texture
    auto textureData = (GLfloat*) malloc(sizeof(GLfloat) * 3 * n);
    double colorScale[3] = {1.0 / 1500.0, 1.15 / 1500.0, 1.66 / 1500.0};
    for (int i = 0; i < 3; i++) {
        for(int ind = 0; ind < n; ind++) {
            textureData[ind * 3 + i] = static_cast<GLfloat>(brdfData[n * i + ind] * colorScale[i]);
        }
    }
    MerlBrdfTexture merlBrdfTexture = MerlBrdfTexture();
    merlBrdfTexture.generate(dimension[0], dimension[1], dimension[2], textureData);
    delete brdfData;
    delete textureData;
    std::cout << "Done." << std::endl;
    return merlBrdfTexture;
}

CubeMapTexture ResourceManager::generateCubeMapTexture(CubeTexParams params) {
    CubeMapTexture cubeMapTexture = CubeMapTexture();
    cubeMapTexture.generate(params);
    return cubeMapTexture;
}