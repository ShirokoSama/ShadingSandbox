//
// Created by Srf on 2019/11/18.
//

#ifndef SANDBOX_QUAD_H
#define SANDBOX_QUAD_H

#include <glad/glad.h>

class Quad {
public:
    Quad();
    ~Quad();
    void RenderQuad();

private:
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
};


#endif //SANDBOX_QUAD_H
