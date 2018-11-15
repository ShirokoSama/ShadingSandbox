//
// Created by Srf on 2017/10/5.
//

#include "Texture2D.h"
#include <iostream>

Texture2D::Texture2D() : width(0), height(0), internal_format(GL_RGB), image_format(GL_RGB), data_format(GL_UNSIGNED_BYTE),
        wrapS(GL_REPEAT), wrapT(GL_REPEAT), filterMin(GL_LINEAR_MIPMAP_LINEAR), filterMax(GL_LINEAR) {
    glGenTextures(1, &this->ID);
}

Texture2D::Texture2D(Tex2DParams params) :
        width(0),
        height(0),
        internal_format(params.internal_format),
        image_format(params.image_format),
        data_format(params.data_format),
        wrapS(params.wrapS),
        wrapT(params.wrapT),
        filterMin(params.filterMin),
        filterMax(params.filterMax) {
    glGenTextures(1, &this->ID);
}

void Texture2D::generate(GLuint width, GLuint height, const GLvoid *data) {
    this->width = width;
    this->height = height;
    // Create Texture
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, this->internal_format, width, height, 0, this->image_format, this->data_format, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filterMax);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::bind() const {
    glBindTexture(GL_TEXTURE_2D, this->ID);
}