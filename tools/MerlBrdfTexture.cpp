//
// Created by Srf on 2018/10/24.
//

#include "MerlBrdfTexture.h"
#include <iostream>

MerlBrdfTexture::MerlBrdfTexture() {
    glGenTextures(1, &this->ID);
}

void MerlBrdfTexture::generate(int dimThetaHalfSize, int dimThetaDiffSize, int dimPhiDiffSize, GLfloat *data) {
    this->dimThetaHalfSize = dimThetaHalfSize;
    this->dimThetaDiffSize = dimThetaDiffSize;
    this->dimPhiDiffSize = dimPhiDiffSize;
    glBindTexture(GL_TEXTURE_3D, this->ID);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, dimPhiDiffSize, dimThetaDiffSize, dimThetaHalfSize, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void MerlBrdfTexture::bind() const {
    glBindTexture(GL_TEXTURE_3D, this->ID);
}
