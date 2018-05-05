#include <cstdio>
#include <cstring>

#include <fstream>
#include <string>

#include <GL/glew.h>
#include <GL/glut.h>

#include <donny/vector_view.hpp>
#include <donny/filesystem.hpp>

namespace donny {
namespace OpenGL {

GLuint BuildShader(const GLchar* pShaderText, const GLenum eShaderType)
{
    GLuint uShaderObj = glCreateShader(eShaderType);
    GLint success = GL_FALSE;
    GLchar sInfoLog[1024];
    
    const int nShaderText = 1;
    const GLchar* pST[nShaderText];
    pST[0] = pShaderText;
    GLint Lengths[nShaderText];
    for (int i = 0; i < nShaderText; ++i)
        Lengths[i] = strlen(pST[i]);
    
    glShaderSource(uShaderObj, nShaderText, pST, Lengths);
    glCompileShader(uShaderObj);
    glGetShaderiv(uShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(uShaderObj, sizeof(sInfoLog), nullptr, sInfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", eShaderType, sInfoLog);
        glDeleteShader(uShaderObj);
        uShaderObj = 0;
    }

    return uShaderObj;
}

bool LinkAndValidateProgram(GLuint uShaderProgram)
{
    GLint success = GL_FALSE;
    GLchar sInfoLog[1024];

    glLinkProgram(uShaderProgram);
    glGetProgramiv(uShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(uShaderProgram, sizeof(sInfoLog), nullptr, sInfoLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", sInfoLog);
        return false;
    }

    glValidateProgram(uShaderProgram);
    glGetProgramiv(uShaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(uShaderProgram, sizeof(sInfoLog), nullptr, sInfoLog);
        fprintf(stderr, "Error validating shader program: '%s'\n", sInfoLog);
        return false;
    }

    fprintf(stdout, "Shaders loaded.\n");

    return true;
}

bool LoadShaders(const GLuint uShaderProgram, const GLchar** pShaderText, const GLenum* eShaderType, int nShaderCount)
{
    for (int i = 0; i < nShaderCount; ++i) {
        GLuint uShaderObj = BuildShader(pShaderText[i], eShaderType[i]);
        glAttachShader(uShaderProgram, uShaderObj);
    }

    return LinkAndValidateProgram(uShaderProgram);
}

GLuint LoadShaders(GLchar const** pShaderText, GLenum const* eShaderType, int nShaderCount)
{
    GLuint uShaderProgram = glCreateProgram();

    if (!LoadShaders(uShaderProgram, pShaderText, eShaderType, nShaderCount))
    {
        glDeleteProgram(uShaderProgram);
        uShaderProgram = 0;
    }

    return uShaderProgram;
}

struct ShaderFileInfo
{
    GLenum type;
    std::string filename;
};

GLuint LoadShaders(donny::vector_view<const ShaderFileInfo> pInfos)
{
    using typename std::ifstream;
    using typename std::ios;

    GLuint uShaderProgram = glCreateProgram();
    GLint success = GL_FALSE;
    GLchar sInfoLog[1024];

    // std::locale utf8_locale(std::locale(), new std::codecvt_utf8<char32_t>);

    for (const ShaderFileInfo &sfi : pInfos) {
        ifstream fshader(sfi.filename, ios::in);
        
        if (!fshader.is_open()) {
            fprintf(stderr, "Fail to open shader file '%s'\n", sfi.filename.c_str());
            continue;
        }

        // fshader.imbue(utf8_locale);
        uintmax_t filesize = donny::filesystem::file_size(fshader);
        GLchar *shadersource = new GLchar[filesize+1];
        shadersource[filesize] = '\0';

        fshader.read(shadersource, filesize);
        // fshader >> shadersource;
        fshader.close();

        GLuint uShaderObj = BuildShader(shadersource, sfi.type);
        glAttachShader(uShaderProgram, uShaderObj);
        glDeleteShader(uShaderObj);

        delete[] shadersource;
    }

    if (!LinkAndValidateProgram(uShaderProgram))
    {
        glDeleteProgram(uShaderProgram);
        uShaderProgram = 0;
    }

    return uShaderProgram;
}

} // OpenGL
} // donny
