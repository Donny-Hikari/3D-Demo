#pragma once

#include <vector>

#include <donny/vector_view.hpp>
#include <donny/logger.hpp>

#include <GL/glew.h>
#include <GL/glut.h>

#include "../lib/vmathex.hpp"
#include "../lib/donny/GLObjects.hpp"

namespace Standard3D {

class StandardObjectsStack
{
public:
    void ensure(int size_)
    {
        const int old_size = _id_max + 1;
        if (old_size >= size_) return;

        VAO.resize(size_);
        VBO.resize(size_);
        EBO.resize(size_);

        glGenVertexArrays(size_ - old_size, VAO.data() + old_size); // Set up the vertex attributes
        glGenBuffers(size_ - old_size, VBO.data() + old_size);  // Set up the vertex array buffer
        glGenBuffers(size_ - old_size, EBO.data() + old_size); // Set up the element array buffer
    }

    int add()
    {
        if (_id_cnt < _id_max) return ++_id_cnt;

        VAO.push_back((GLuint)0);
        VBO.push_back((GLuint)0);
        EBO.push_back((GLuint)0);

        glGenVertexArrays(1, &*VAO.rbegin()); // Set up the vertex attributes
        glGenBuffers(1, &*VBO.rbegin());  // Set up the vertex array buffer
        glGenBuffers(1, &*EBO.rbegin()); // Set up the element array buffer

        return ++_id_cnt;
    }
    void remove(int id)
    {
        glGenVertexArrays(1, &VAO[id]); // Set up the vertex attributes
        glGenBuffers(1, &VBO[id]);  // Set up the vertex array buffer
        glGenBuffers(1, &EBO[id]); // Set up the element array buffer
        VAO[id] = 0;
        VBO[id] = 0;
        EBO[id] = 0;
    }

    GLuint getVAO(int id) const { return VAO[id]; }
    GLuint getVBO(int id) const { return VBO[id]; }
    GLuint getEBO(int id) const { return EBO[id]; }

private:
    int _id_max = -1;
    int _id_cnt = -1;

    friend class StandardObject;

    StandardObjectsStack() { }
    ~StandardObjectsStack() { }

    std::vector<GLuint> VAO;
    std::vector<GLuint> VBO;
    std::vector<GLuint> EBO;
};

class StandardObject : public donny::OpenGL::Object
{
public:
    StandardObject() :
        mId(objectsStack.add()),
        mShaderProgram(0)
    {
    }

    virtual bool onInitialize() = 0;
    virtual bool onDraw() = 0;

    virtual bool initialize() override
    {
        bool result = false;
        glBindVertexArray(objectsStack.getVAO(mId));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectsStack.getEBO(mId));
        glBindBuffer(GL_ARRAY_BUFFER, objectsStack.getVBO(mId));
        result = onInitialize();
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return result;
    }

    virtual bool draw() const override
    {
        bool result = false;
        glUseProgram(mShaderProgram);
        glBindVertexArray(objectsStack.getVAO(mId));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objectsStack.getEBO(mId));
        result = const_cast<StandardObject*>(this)->onDraw();
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glUseProgram(0);
        return result;
    }

    uint getId() const { return mId; }
    GLuint getVAO() const { return objectsStack.getVAO(mId); }
    GLuint getVBO() const { return objectsStack.getVBO(mId); }
    GLuint getEBO() const { return objectsStack.getEBO(mId); }
    GLuint getShaderProgram() const { return mShaderProgram; }

    GLuint setShaderProgram(GLuint newShaderProgram)
    {
        GLuint oldShaderProgram = mShaderProgram;
        mShaderProgram = newShaderProgram;
        return oldShaderProgram;
    }

private:
    typedef class Object base;
    static StandardObjectsStack objectsStack;
    
    uint mId;
    GLuint mShaderProgram;
};
StandardObjectsStack StandardObject::objectsStack;


// void calculateNormalEAO(donny::vector_view<const vmath::vec3> positions,
//                         donny::vector_view<const GLushort> indices,
//                         donny::vector_view<vmath::vec3> &normals)
// {
//     int sign = 1;
//     int ii = 0;
//     while (ii + 2 < indices.size())
//     {
//         vmath::vec3 v = positions[ii+1] - positions[ii];
//         vmath::vec3 u = positions[ii+1] - positions[ii+2];
//         vmath::vec3 n = vmath::cross(v, u);
//         normals[ii/3] = n;
//         ii += 3;
//     }
// }

// Calculate Element Array's Normals
bool calculateEANormals(donny::vector_view<const GLfloat> positions,
                        donny::vector_view<const GLushort> indices,
                        donny::vector_view<GLfloat> normals,
                        GLsizei stride,
                        GLuint restartInd)
{
    using namespace donny;

    if (stride < 3) return false; // Failed

    const int length = positions.size() / stride;
    std::vector<vmath::vec3> v3Pos; v3Pos.resize(length);
    for (int a = 0; a < v3Pos.size(); ++a)
    {
        v3Pos[a] = vmath::vec3(positions[a*stride],
                               positions[a*stride+1],
                               positions[a*stride+2]);
    }

    int sign = -1;
    int ii = 0;
    std::vector<vmath::vec3> v3Normal; v3Normal.resize(length);
    // std::vector<int> nRepeatCnt; nRepeatCnt.resize(length);
    for (vmath::vec3 &v3Norm : v3Normal) {
        v3Norm = vmath::vec3(0.f, 0.f, 0.f);
    }
    while (ii + 2 < indices.size())
    {
        if (indices[ii] == restartInd || indices[ii+1] == restartInd || indices[ii+2] == restartInd) {
            ++ii;
            sign = -1;
            continue;
        }

        int inds[3] = { indices[ii], indices[ii+1], indices[ii+2] };
        for (int &ind : inds) {
            int p = (&ind - &inds[0]);
            vmath::vec3 v = v3Pos[inds[(p+2)%3]] - v3Pos[inds[p]];
            vmath::vec3 u = v3Pos[inds[(p+1)%3]] - v3Pos[inds[p]];
            // vmath::vec3 n = vmath::normalize(vmath::cross(v, u) * sign);
            vmath::vec3 n = vmath::cross(v, u) * sign;
            float sin_alpha = vmath::length(n) / (vmath::length(v) * vmath::length(u));

            // logstdout << inds[(p+2)%3] << inds[p] << inds[(p+1)%3] << endl;
            // logstdout << ii << ' ' << asin(sin_alpha) / 3.141592657 * 180 << endl;
            // logstdout << v3Pos[inds[1]] << endl;
            // logstdout << v3Pos[inds[0]] << endl;
            // logstdout << v << endl;
            // logstdout << u << endl;
            // logstdout << endl << n << endl;

            v3Normal[ind] += n * asin(sin_alpha);
            // ++nRepeatCnt[ind];
            // logstdout << ind << ' ' << nRepeatCnt[ind] << endl;
            // logstdout << ind << ' ' << v3Normal[ind] << endl;
        }

        ++ii;
        sign *= -1;
    }

    for (int a = 0; a < v3Pos.size(); ++a)
    {
        // v3Normal[a] = vmath::normalize(v3Normal[a] / nRepeatCnt[a]);
        v3Normal[a] = vmath::normalize(v3Normal[a]);
        normals[a*stride+0] = v3Normal[a][0];
        normals[a*stride+1] = v3Normal[a][1];
        normals[a*stride+2] = v3Normal[a][2];
        if (stride == 4) normals[a*stride+3] = 1.0f;
    }

    return true;
}

} // Standard3D
