#pragma once

#include <vector>

#include <GL/glew.h>
#include <GL/glut.h>

namespace donny {
namespace OpenGL {

class Object
{
public:
    virtual bool initialize() = 0;
    virtual bool draw() const = 0;
};

} // OpenGL
} // donny
