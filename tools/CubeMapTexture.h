//
// Created by Srf on 2018/11/14.
//

#ifndef CG_LAB3_CUBEMAPTEXURE_H
#define CG_LAB3_CUBEMAPTEXURE_H

#include <glad/glad.h>

struct CubeTexParams {
    GLsizei size;
    GLuint internal_format;
    GLuint image_format;
    GLuint data_type;
    GLuint wrap;
    GLuint filterMin;
    GLuint filterMax;
};

class CubeMapTexture {
public:
    CubeMapTexture();
    void generate(CubeTexParams params);
    void bind() const;
    GLuint ID;
};


#endif //CG_LAB3_CUBEMAPTEXURE_H
