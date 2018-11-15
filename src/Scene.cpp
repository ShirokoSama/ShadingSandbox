//
// Created by Srf on 2018/10/20.
//

#include "Scene.h"

Scene::~Scene() {
    delete camera;
    delete skybox;
    for (auto& rto : rtObjects)
        rto.mesh.Clear();
}

void Scene::init() {
    // init skybox
    this->skybox = new Skybox("resource/hdr/newport_loft.hdr");

    // get the relevant block indices in each shader object
    // and link each shader's uniform block to this uniform binding point
    for (auto& rto : this->rtObjects) {
        GLuint vpUniformBlockIndex = glGetUniformBlockIndex(rto.shader.ID, "Matrices");
        glUniformBlockBinding(rto.shader.ID, vpUniformBlockIndex, 0);
        GLuint lightsUniformBlockIndex = glGetUniformBlockIndex(rto.shader.ID, "Lights");
        glUniformBlockBinding(rto.shader.ID, lightsUniformBlockIndex, 1);
    }
    Shader skyboxShader = ResourceManager::getShader("skyboxShader");
    GLuint vpUniformBlockIndex = glGetUniformBlockIndex(skyboxShader.ID, "Matrices");
    glUniformBlockBinding(skyboxShader.ID, vpUniformBlockIndex, 0);

    // generate uniform buffer object for global blocks
    // bind position 0
    // view matrix
    // projection matrix
    // bind position 1
    // pointLights
    // dirLights
    glGenBuffers(1, &uboGlobalBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalBlock);

    // calculate offset alignment
    GLint offsetAlignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offsetAlignment);
    // calculate min size of buffer0
    GLint sizeOfBuffer0 = static_cast<GLint>(ceil(static_cast<double>(2 * sizeof(glm::mat4)) / offsetAlignment)) * offsetAlignment;
    glBufferData(
            GL_UNIFORM_BUFFER,
            // 2 glm::mat4 + pointLight structure array + dirLight structure array
            sizeOfBuffer0 + this->pointLights.size() * 48 + this->dirLights.size() * 32,
            nullptr, GL_STATIC_DRAW
            );
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboGlobalBlock, 0, 2 * sizeof(glm::mat4));
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, uboGlobalBlock,
            sizeOfBuffer0, this->pointLights.size() * 48 + this->dirLights.size() * 32);
    if (!this->pointLights.empty())
        glBufferSubData(
                GL_UNIFORM_BUFFER,
                sizeOfBuffer0,
                this->pointLights.size() * 48,
                &this->pointLights[0]
                );
    if (!this->dirLights.empty())
        glBufferSubData(
                GL_UNIFORM_BUFFER,
                sizeOfBuffer0 + this->pointLights.size() * 48,
                this->dirLights.size() * 32,
                &this->dirLights[0]
                );
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::Draw() {
    glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalBlock);
    glm::mat4 view = this->camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(this->camera->Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    for (auto& rto : rtObjects) {
        rto.shader.setVector3f("viewPos", camera->Position, true);
		rto.shader.use();
		glActiveTexture(GL_TEXTURE2);
		this->skybox->BindEnvironmentCubeMap();
        rto.mesh.DrawMesh(&rto.shader);
    }
    this->skybox->Draw();
    assert(glGetError() == GL_NO_ERROR);
}

void Scene::Update(float deltaTime) {
    for (auto& rto : rtObjects)
        rto.mesh.Update(deltaTime);
}

void Scene::ProcessKeyBoard(MaterialControl key, float deltaTime) {
    if (!rtObjects.empty())
        rtObjects[0].mesh.processMaterialControl(key, deltaTime);
}