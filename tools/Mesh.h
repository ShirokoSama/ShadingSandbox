//
// Created by Srf on 2018/10/20.
//

#ifndef CG_LAB2_MESH_H
#define CG_LAB2_MESH_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "Shader.h"
#include "Texture2D.h"
#include "common.h"
#include "MerlBrdfTexture.h"

struct OBJVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

enum MaterialType {
    BASIC,
    MEASURED,
    MICROFACET
};

enum MaterialControl {
    ROUGHNESS_INCREASE,
    ROUGHNESS_DECREASE,
    KD_INCREASE,
    KD_DECREASE
};

class Mesh {
public:
    Mesh (const std::string &objFilePath, glm::mat4 modelMatrix) : modelMatrix(modelMatrix) {

        using namespace std;
        // read and format obj file into vertices and indices
        ifstream fin;
        fin.open(objFilePath);
        if (fin.fail())
            throw "open wavefront object file fail";
        cout << "Loading \"" << objFilePath << "\" .. ";
        cout.flush();

        vector<glm::vec3> positionBuffers;
        vector<glm::vec2> texcoordBuffers;
        vector<glm::vec3> normalBuffers;
        typedef unordered_map<VertexInfoIndex, GLuint, VertexInfoIndexHash> VertexInfoMap;
        VertexInfoMap vertexInfoMap;

        string line;
        while (getline(fin, line)) {
            istringstream lineStream(line);
            string prefix;
            lineStream >> prefix;
            if (prefix == "v") {
                glm::vec3 position(0.0f);
                lineStream >> position.x >> position.y >> position.z;
                positionBuffers.push_back(position);
            }
            else if (prefix == "vt") {
                glm::vec2 texcoord(0.0f);
                lineStream >> texcoord.x >> texcoord.y;
                texcoordBuffers.push_back(texcoord);
            }
            else if (prefix == "vn") {
                glm::vec3 normal(0.0f);
                lineStream >> normal.x >> normal.y >> normal.z;
                normalBuffers.push_back(normal);
            }
            else if (prefix == "f") {
                string v1, v2, v3, v4;
                lineStream >> v1 >> v2 >> v3 >> v4;
                VertexInfoIndex verts[2][3];
                int triangleNumPerFace = 1;
                verts[0][0] = VertexInfoIndex(v1);
                verts[0][1] = VertexInfoIndex(v2);
                verts[0][2] = VertexInfoIndex(v3);
                // face is a quad, and then we need to split this quad to two triangles
                if (!v4.empty()) {
                    verts[1][0] = VertexInfoIndex(v4);
                    verts[1][1] = verts[0][0];
                    verts[1][2] = verts[0][2];
                    triangleNumPerFace = 2;
                }
                for (int i = 0; i < triangleNumPerFace; i++) {
                    for (int j = 0; j < 3; j++) {
                        const VertexInfoIndex &vInfoIndex = verts[i][j];
                        VertexInfoMap::const_iterator it = vertexInfoMap.find(vInfoIndex);
                        if (it == vertexInfoMap.end()) {
                            vertexInfoMap[vInfoIndex] = (GLuint)this->vertices.size();
                            this->indices.push_back((GLuint)this->vertices.size());

                            // start build OBJVertex and insert into vertices array
                            glm::vec3 vp = positionBuffers.at(vInfoIndex.p - 1);
                            glm::vec3 vn;
                            if (normalBuffers.empty()) {
                                glm::vec3 p0 = positionBuffers.at(verts[i][0].p - 1);
                                glm::vec3 p1 = positionBuffers.at(verts[i][1].p - 1);
                                glm::vec3 p2 = positionBuffers.at(verts[i][2].p - 1);
                                vn = glm::normalize(glm::cross(p1 - p0, p2 - p0));
                            }
                            else
                                vn = normalBuffers.at(vInfoIndex.n - 1);
                            glm::vec2 vt(0.0f);
                            if (!texcoordBuffers.empty())
                                vt = texcoordBuffers.at(vInfoIndex.uv - 1);
                            this->vertices.push_back(OBJVertex{vp, vn, vt});
                        }
                        else {
                            this->indices.push_back(it->second);
                        }
                    }
                }
            }
        }

        fin.close();
        cout << "Done." << endl;
        cout << "vertices size is " << this->vertices.size() << " and indices size is " << this->indices.size() << endl;

        // initialize vertices and indices data into opengl
        SetupMesh();
        assert(glGetError() == GL_NO_ERROR);
    }

    ~Mesh() = default;

    void SetTexture(Texture2D tex) {
        this->texture = tex;
        this->mType = BASIC;
    }

    void SetBrdf(MerlBrdfTexture brdf) {
        this->brdf = brdf;
        this->mType = MEASURED;
    }

    void SetRoughness(float roughness) {
        this->roughnesss = roughness;
        this->mType = MICROFACET;
    }

    void SetKd(glm::vec3 kd) {
        this->kd = kd;
        this->mType = MICROFACET;
    }

    void DrawMesh(Shader *shader);
    void Update(float deltaTime) { }
    void processMaterialControl(MaterialControl key, float deltaTime);
    void Clear();

private:
    struct VertexInfoIndex {
        uint32_t p = (uint32_t) -1;
        uint32_t n = (uint32_t) -1;
        uint32_t uv = (uint32_t) -1;

        inline VertexInfoIndex() = default;

        explicit  inline VertexInfoIndex(const std::string &str) {
            using namespace std;
            vector<string> tokens = tokenize(str, "/", true);
            if (tokens.empty() || tokens.size() > 3)
                throw "Invalid vertex indices data";
            p = toUInt(tokens[0]);
            if (tokens.size() >= 2 && !tokens[1].empty())
                uv = toUInt(tokens[1]);
            if (tokens.size() >= 3 && !tokens[2].empty())
                n = toUInt(tokens[2]);
        }

        inline bool operator==(const VertexInfoIndex &v) const {
            return v.p == p && v.n == n && v.uv == uv;
        }
    };

    /// Hash function for VertexIndexInfo
    struct VertexInfoIndexHash : std::unary_function<VertexInfoIndex, size_t> {
        std::size_t operator()(const VertexInfoIndex &v) const {
            size_t hash = std::hash<uint32_t>()(v.p);
            hash = hash * 37 + std::hash<uint32_t>()(v.uv);
            hash = hash * 37 + std::hash<uint32_t>()(v.n);
            return hash;
        }
    };

    // setup VAO and bind VBO, EBO
    void SetupMesh();

    GLuint meshVAO{}, VBO{}, EBO{};
    std::vector<OBJVertex> vertices;
    std::vector<GLuint> indices;
    glm::mat4 modelMatrix;

    Texture2D texture;
    MerlBrdfTexture brdf;
    MaterialType mType = BASIC;
    float roughnesss = 0.5f;
    glm::vec3 kd = glm::vec3(0.0f);
};


#endif //CG_LAB2_MESH_H
