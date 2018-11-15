//
// Created by Srf on 2018/10/24.
//

#ifndef CG_LAB2_MERLBRDFTEXTURE_H
#define CG_LAB2_MERLBRDFTEXTURE_H

#include <glad/glad.h>

class MerlBrdfTexture {
public:
    MerlBrdfTexture();
    void generate(int dimThetaHalfSize, int dimThetaDiffSize, int dimPhiDiffSize, GLfloat *data);
    void bind() const;
    GLuint ID;
    int dimThetaHalfSize, dimThetaDiffSize, dimPhiDiffSize;
};


#endif //CG_LAB2_MERLBRDFTEXTURE_H
