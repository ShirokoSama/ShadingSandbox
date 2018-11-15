//
// Created by Srf on 2018/10/20.
//

#ifndef CG_LAB2_SCENE_H
#define CG_LAB2_SCENE_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include "../tools/Camera.h"
#include "../tools/ResourceManager.h"
#include "../tools/Mesh.h"
#include "Skybox.h"

// screen size
const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

struct DirLight {
    // size in std140 is 32
    glm::vec4 direction;        // 16 0
    glm::vec4 radiance;         // 16 16
};

struct PointLight {
    // size in std140 is 48
    // size in std140 is 48
    glm::vec4 position;         // 16 0
    glm::vec4 radiance;         // 16 16
    GLfloat constant;           // 4 32
    GLfloat linear;             // 4 36
    GLfloat quadratic;          // 4 40
    GLfloat placeHolder;        // 4 44
};

struct RenderTargetObject {
    Mesh mesh;
    Shader shader;
};

class Scene {
public:
    explicit Scene(const std::string &configFilePath) {
        using namespace rapidjson;

        // read json string from config file
        std::ifstream fin;
        fin.open(configFilePath);
        if (fin.fail())
            throw "open config file fail";
        std::string json((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();

        // parse json by rapidjson's dom
        Document dom;
        char *jsonTextBuffer = (char*)malloc(sizeof(char) * (json.length() + 1));
        sprintf(jsonTextBuffer, "%s", json.c_str());
        std::cout << jsonTextBuffer << std::endl;
        if (dom.ParseInsitu(jsonTextBuffer).HasParseError())
            throw "json parse error";
        assert(dom.IsObject());
        assert(dom.HasMember("scene"));

        // parse camera attributes
        const Value& cameraPosition = Pointer("/scene/camera/position").GetWithDefault(dom, "0.0 0.0 0.0");
        const Value& cameraSpeed = Pointer("/scene/camera/movement_speed").GetWithDefault(dom, SPEED);
        const Value& cameraSensitivity = Pointer("/scene/camera/mouse_sensitivity").GetWithDefault(dom, SENSITIVITY);
        camera = new Camera(
                parseVector3FromString(cameraPosition.GetString()),
                static_cast<GLfloat>(cameraSpeed.GetDouble()),
                static_cast<GLfloat>(cameraSensitivity.GetDouble())
                );
        std::cout << "Init camera done." << std::endl;

        // parse point lights attributes
        const Value *pointLightArray = Pointer("/scene/point_lights").Get(dom);
        if (pointLightArray) {
            for (auto& pointLightValue : pointLightArray->GetArray()) {
                PointLight pl = {
                        parseVector4FromString(pointLightValue["position"].GetString()),
                        parseVector4FromString(pointLightValue["radiance"].GetString()),
                        static_cast<GLfloat>(pointLightValue["constant"].GetDouble()),
                        static_cast<GLfloat>(pointLightValue["linear"].GetDouble()),
                        static_cast<GLfloat>(pointLightValue["quadratic"].GetDouble()),
                        0.0f
                };
                pointLights.push_back(pl);
            }
        }
        std::cout << "Init " << pointLights.size() << " point lights done." << std::endl;

        // parse direction light attributes
        const Value *directionLightArray = Pointer("/scene/direction_lights").Get(dom);
        if (directionLightArray) {
            for (auto& directionLightValue : directionLightArray->GetArray()) {
                DirLight dl = {
                        parseVector4FromString(directionLightValue["direction"].GetString()),
                        parseVector4FromString(directionLightValue["radiance"].GetString())
                };
                dirLights.push_back(dl);
            }
        }
        std::cout << "Init " << dirLights.size() << " direction lights done." << std::endl;

        // generate solid texture
        ResourceManager::loadSolidTexture(glm::vec3(1.0f), "white");
        // parse meshes and shaders
        const Value *objectArray = Pointer("/scene/objects").Get(dom);
        if (objectArray) {
            int count = 0;
            for (auto& objectValue : objectArray->GetArray()) {
                const std::string objFilePath = objectValue["obj_file_path"].GetString();
                const std::string vsFilePath = objectValue["vertex_shader_file_path"].GetString();
                const std::string fsFilePath = objectValue["fragment_shader_file_path"].GetString();
                const std::string resourceID = std::to_string(count);
                count++;

                // create shader object from shader file
                Shader shader = ResourceManager::loadShader(vsFilePath.c_str(), fsFilePath.c_str(), nullptr, resourceID);

                // create model matrix from transform, rotate and scale operation
                glm::mat4 model(1.0f);
                model = glm::translate(model, parseVector3FromString(objectValue["position"].GetString()));
                model = glm::rotate(model, glm::radians(static_cast<GLfloat>(objectValue["rotate_y"].GetDouble())), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(static_cast<GLfloat>(objectValue["rotate_x"].GetDouble())), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(static_cast<GLfloat>(objectValue["rotate_z"].GetDouble())), glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, glm::vec3(parseVector3FromString(objectValue["scale"].GetString())));

                // load mesh object from wavefront object file
                Mesh mesh(objFilePath, model);

                const Value *brdfPathValue = Pointer("/material/brdf_file_path").Get(objectValue);
                if (brdfPathValue == nullptr) {
                    const Value *roughnessValue = Pointer("/material/roughness").Get(objectValue);
                    if (roughnessValue != nullptr) {
                        // read pbr attributes from config file
                        GLfloat roughness = roughnessValue->GetFloat();
                        const Value *fresnelValue = Pointer("/material/fresnel_0").Get(objectValue);
                        glm::vec3 fresnel_0 = parseVector3FromString(fresnelValue->GetString());
                        const Value *kdValue = Pointer("/material/kd").Get(objectValue);
                        glm::vec3 kd = parseVector3FromString(kdValue->GetString());
                        const Value *texturePathValue = Pointer("/material/texture_file_path").Get(objectValue);
                        if (texturePathValue != nullptr) {
                            std::string texturePath = texturePathValue->GetString();
                            Texture2D texture = ResourceManager::loadTexture(texturePath.c_str(), GL_FALSE, texturePath);
                            mesh.SetTexture(texture);
                        }
                        else {
                            Texture2D texture = ResourceManager::getTexture("white");
                            mesh.SetTexture(texture);
                        }
                        mesh.SetRoughness(roughness);
                        mesh.SetKd(kd);
                        {
                            shader.use();
                            shader.setInteger("texSlot", 0);
                            shader.setMatrix4("model", model);
                            shader.setFloat("alpha", roughness);
                            shader.setVector3f("fresnel_0", fresnel_0);
                            shader.setVector3f("kd", kd);
                        }
                    }
                    else {
                        // read phong or blinn-phong material info from config file
                        const Value *kaValue = Pointer("/material/ka").Get(objectValue);
                        glm::vec3 ka = parseVector3FromString(kaValue->GetString());
                        const Value *kdValue = Pointer("/material/kd").Get(objectValue);
                        glm::vec3 kd = parseVector3FromString(kdValue->GetString());
                        const Value *ksValue = Pointer("/material/ks").Get(objectValue);
                        glm::vec3 ks = parseVector3FromString(ksValue->GetString());
                        const Value *glossinessValue = Pointer("/material/glossiness").Get(objectValue);
                        int glossiness = glossinessValue->GetInt();
                        const Value *texturePathValue = Pointer("/material/texture_file_path").Get(objectValue);
                        if (texturePathValue != nullptr) {
                            std::string texturePath = texturePathValue->GetString();
                            Texture2D texture = ResourceManager::loadTexture(texturePath.c_str(), GL_FALSE, texturePath);
                            mesh.SetTexture(texture);
                        }
                        else {
                            Texture2D texture = ResourceManager::getTexture("white");
                            mesh.SetTexture(texture);
                        }
                        // Initialization here is not appropriate but we assume model and texture position are constant in this scene
                        {
                            shader.use();
                            shader.setInteger("texSlot", 0);
                            shader.setMatrix4("model", model);
                            shader.setVector3f("ka", ka);
                            shader.setVector3f("kd", kd);
                            shader.setVector3f("ks", ks);
                            shader.setInteger("glossiness", glossiness);
                        }
                    }
                }
                else {
                    std::string brdfPath = brdfPathValue->GetString();
                    MerlBrdfTexture brdfTexture = ResourceManager::loadMerlBrdfTexture(brdfPath.c_str(), brdfPath);
                    mesh.SetBrdf(brdfTexture);
                    {
                        shader.use();
                        shader.setInteger("merlBrdf", 1);
                        shader.setMatrix4("model", model);
                        shader.setInteger("THETA_HALF_SAMPLING_SIZE", static_cast<GLint>(brdfTexture.dimThetaHalfSize));
                        shader.setInteger("THETA_DIFF_SAMPLING_SIZE", static_cast<GLint>(brdfTexture.dimThetaDiffSize));
                        shader.setInteger("PHI_DIFF_SAMPLING_SIZE", static_cast<GLint>(brdfTexture.dimPhiDiffSize));
                    }
                };

				shader.setInteger("environmentMap", 2);
                RenderTargetObject rto = {mesh, shader};
                rtObjects.push_back(rto);
            }
        }
        std::cout << "Init " << rtObjects.size() << " render target objects done." << std::endl;

        delete jsonTextBuffer;

        // init render scene and uniform attributes
        init();
        assert(glGetError() == GL_NO_ERROR);
    }

    ~Scene();

    Camera *GetCameraPointer() { return camera; }
    void Draw();
    void Update(float deltaTime);
    void ProcessKeyBoard(MaterialControl key, float deltaTime);

private:
    void init();

    Camera *camera;
    Skybox *skybox;
    std::vector<DirLight> dirLights;
    std::vector<PointLight> pointLights;
    std::vector<RenderTargetObject> rtObjects;

    GLuint uboGlobalBlock{};
};


#endif //CG_LAB2_SCENE_H