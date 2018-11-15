//
// Created by Srf on 2018/11/14.
//

#include "CubeMapTexture.h"

CubeMapTexture::CubeMapTexture() {
    glGenTextures(1, &ID);
}

void CubeMapTexture::generate(CubeTexParams params) {
    this->bind();
    for (unsigned int i = 0; i < 6; i++)
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                params.internal_format,
                params.size,
                params.size,
                0,
                params.image_format,
                params.data_type,
                nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, params.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, params.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, params.wrap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, params.filterMin);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, params.filterMax);
}

void CubeMapTexture::bind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->ID);
}