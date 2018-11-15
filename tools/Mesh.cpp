//
// Created by Srf on 2018/10/20.
//

#include "Mesh.h"

void Mesh::SetupMesh() {
    // generate vertex arrays and buffers
    glGenVertexArrays(1, &meshVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind VAO, VBO, EBO and
    glBindVertexArray(meshVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(OBJVertex), &this->vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);
    // set vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), (void*) nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), (void*) offsetof(OBJVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), (void*) offsetof(OBJVertex, texcoord));
    // unbind VAO
    glBindVertexArray(0);
}

void Mesh::DrawMesh(Shader *shader) {
    shader->use();
    if (this->mType == MICROFACET) {
        shader->setFloat("alpha", this->roughnesss);
        shader->setVector3f("kd", this->kd);
    }
    // Assume texture position and model matrix are constant in this scene
//    shader->setInteger("texSlot", 0);
//    shader->setMatrix4("model", modelMatrix);
    switch (this->mType) {
        case BASIC:
            glActiveTexture(GL_TEXTURE0);
            texture.bind();
            break;
        case MEASURED:
            glActiveTexture(GL_TEXTURE1);
            brdf.bind();
            break;
        default:
            glActiveTexture(GL_TEXTURE0);
            texture.bind();
    }
    glBindVertexArray(meshVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::Clear() {
    glDeleteVertexArrays(1, &this->meshVAO);
    glDeleteBuffers(1, &this->VBO);
    glDeleteBuffers(1, &this->EBO);
}

void Mesh::processMaterialControl(MaterialControl key, float deltaTime) {
    if (this->mType == MICROFACET) {
        switch (key) {
            case ROUGHNESS_INCREASE:
                this->roughnesss = std::min(1.0f, roughnesss + 0.1f * deltaTime);
                break;
            case ROUGHNESS_DECREASE:
                this->roughnesss = std::max(0.0f, roughnesss - 0.1f * deltaTime);
                break;
            case KD_INCREASE:
                this->kd.x = std::min(1.0f, kd.x + 0.1f * deltaTime);
                this->kd.y = std::min(1.0f, kd.y + 0.1f * deltaTime);
                this->kd.z = std::min(1.0f, kd.z + 0.1f * deltaTime);
                break;
            case KD_DECREASE:
                this->kd.x = std::max(0.0f, kd.x - 0.1f * deltaTime);
                this->kd.y = std::max(0.0f, kd.y - 0.1f * deltaTime);
                this->kd.z = std::max(0.0f, kd.z - 0.1f * deltaTime);
                break;
        }
    }
}