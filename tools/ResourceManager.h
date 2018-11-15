//
// Created by Srf on 2017/10/5.
//

#ifndef BREAKOUT_RESOURCEMANAGER_H
#define BREAKOUT_RESOURCEMANAGER_H

#include <unordered_map>
#include <string>
#include <glad/glad.h>
#include "Texture2D.h"
#include "MerlBrdfTexture.h"
#include "Shader.h"
#include "CubeMapTexture.h"

class ResourceManager {
public:
    static Shader loadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name);
    static Shader getShader(std::string name);
    static Texture2D loadTexture(const GLchar *file, GLboolean alpha, std::string name, Tex2DParams params = {false});
    // generate a texture which is a single point with one color
    static Texture2D loadSolidTexture(glm::vec3 color, std::string name);
    // load a merl brdf texture from file
    static MerlBrdfTexture loadMerlBrdfTexture(const GLchar *file, std::string name);
    static CubeMapTexture loadCubeMapTexture(CubeTexParams params, std::string name);
    static Texture2D getTexture(std::string name);
    static void clear();
private:
    ResourceManager() = default;
    static Shader loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile = nullptr);
    static Texture2D loadTextureFromFile(const GLchar *file, GLboolean alpha, Tex2DParams params);
    static Texture2D generateSolidTexture(glm::vec3 color);
    static MerlBrdfTexture loadMerlBrdfFromFile(const GLchar *file);
    static CubeMapTexture generateCubeMapTexture(CubeTexParams params);

    static std::unordered_map<std::string, Shader> shaders;
    static std::unordered_map<std::string, Texture2D> textures;
    static std::unordered_map<std::string, MerlBrdfTexture> merlBrdfs;
    static std::unordered_map<std::string, CubeMapTexture> cubeMaps;
};


#endif //BREAKOUT_RESOURCEMANAGER_H
