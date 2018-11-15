//
// Created by Srf on 2017/10/5.
//

#ifndef BREAKOUT_TEXTURE2D_H
#define BREAKOUT_TEXTURE2D_H

#include <glad/glad.h>

struct Tex2DParams {
    bool use;   // default use param is false for function overuse
    GLuint internal_format;
    GLuint image_format;
    GLuint data_format;
    GLuint wrapS;
    GLuint wrapT;
    GLuint filterMin;
    GLuint filterMax;
};

class Texture2D {
public:
    GLuint ID;
    GLuint width, height; // Width and height of loaded image in pixels
    GLuint internal_format; // Format of texture object
    GLuint image_format; // Format of loaded image
    GLuint data_format; // Format of pixel data
    GLuint wrapS; // Wrapping mode on S axis
    GLuint wrapT; // Wrapping mode on T axis
    GLuint filterMin; // Filtering mode if texture pixels < screen pixels
    GLuint filterMax; // Filtering mode if texture pixels > screen pixels
    // Constructor (sets default texture modes)
    Texture2D();
    explicit Texture2D(Tex2DParams params);
    // Generates texture from image data
    void generate(GLuint width, GLuint height, const GLvoid* data);
    // Binds the texture as the current active GL_TEXTURE_2D texture object
    void bind() const;
};


#endif //BREAKOUT_TEXTURE2D_H
